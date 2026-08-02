/*
 * economy.h - Headless economy calculations
 *
 * Every function here is pure: it reads its inputs from parameters and touches
 * no global or member state, no terminal, and no clock. That is what makes the
 * game's money math unit testable.
 *
 * These formulas previously lived as member functions on GamblingGame, where
 * they read ~15 different members directly and could only be exercised by
 * playing the game by hand. The member functions still exist as one-line
 * delegations so that call sites did not have to change.
 */

#ifndef CORE_ECONOMY_H
#define CORE_ECONOMY_H

#include <vector>
#include <cmath>
#include <climits>

#include "generator.h"
#include "upgrade.h"

namespace economy {

// ============================================================================
// UPGRADES
// ============================================================================

/*
 * defaultUpgradeValue() - Base value for an upgrade type when none is present.
 *
 * These match the "Base:" comments in initializeUpgrades().
 *
 * Note: the original getUpgradeValue() had `default: return 0.0` covering the
 * four blackjack/roulette types. That branch is unreachable today because
 * initializeUpgrades() registers all ten types, but a 0.0 *payout multiplier*
 * means a win pays nothing, so a new type added without initialization would
 * silently pay out zero. The payout types below default to their documented
 * base multipliers instead of 0.0.
 */
inline double defaultUpgradeValue(UpgradeType type) {
    switch (type) {
        case UpgradeType::WHEEL_CHANCE:     return 50.0;
        case UpgradeType::WHEEL_PAYOUT:     return 2.0;
        case UpgradeType::SLOTS_CHANCE:     return 16.67;
        case UpgradeType::SLOTS_PAYOUT:     return 5.0;
        case UpgradeType::INCOME_BOOST:     return 100.0;
        case UpgradeType::LUCKY_BONUS:      return 0.0;
        case UpgradeType::BLACKJACK_PAYOUT: return 2.0;
        case UpgradeType::DEALER_BUST:      return 0.0;
        case UpgradeType::ROULETTE_PAYOUT:  return 2.0;
        case UpgradeType::LUCKY_NUMBER:     return 0.0;
    }
    return 0.0;
}

/*
 * upgradeValue() - Current effect value of a given upgrade type.
 *
 * Falls back to defaultUpgradeValue() when the upgrade is absent from the list.
 */
inline double upgradeValue(const std::vector<Upgrade>& upgrades, UpgradeType type) {
    for (std::size_t i = 0; i < upgrades.size(); ++i) {
        if (upgrades[i].getType() == type) {
            return upgrades[i].getCurrentValue();
        }
    }
    return defaultUpgradeValue(type);
}

/*
 * incomeMultiplier() - Passive income scale factor from the Income Boost upgrade.
 *
 * INCOME_BOOST is stored as a percentage (100 = no change), so this is 1.0 at
 * base and 2.0 when fully upgraded.
 */
inline double incomeMultiplier(const std::vector<Upgrade>& upgrades) {
    return upgradeValue(upgrades, UpgradeType::INCOME_BOOST) / 100.0;
}

/*
 * allUpgradesMaxed() - True when every upgrade is at its maximum level.
 *
 * An empty list is not "maxed"; that would hand out the achievement on a fresh
 * save before any upgrades are initialized.
 */
inline bool allUpgradesMaxed(const std::vector<Upgrade>& upgrades) {
    if (upgrades.empty()) return false;
    for (std::size_t i = 0; i < upgrades.size(); ++i) {
        if (!upgrades[i].isMaxed()) return false;
    }
    return true;
}

// ============================================================================
// ACHIEVEMENTS AND PERKS
// ============================================================================

/*
 * achievementMultiplier() - Income bonus from unlocked achievements (+1% each).
 */
inline double achievementMultiplier(int unlockedCount) {
    if (unlockedCount < 0) unlockedCount = 0;
    return 1.0 + (unlockedCount * 0.01);
}

/*
 * passiveBoostFromPerk() - Prestige perk passive income boost (+5% per level).
 */
inline double passiveBoostFromPerk(int perkLevel) {
    if (perkLevel < 0) perkLevel = 0;
    return 1.0 + (perkLevel * 0.05);
}

/*
 * startingMoneyFromPerk() - Balance granted after a prestige (+$50 per level).
 */
inline double startingMoneyFromPerk(int perkLevel) {
    if (perkLevel < 0) perkLevel = 0;
    return 100.0 + (perkLevel * 50.0);
}

/*
 * generatorCostMultiplier() - Generator discount from a prestige perk (-5%/level).
 *
 * Clamped so a hand-edited save cannot drive the multiplier to zero or negative,
 * which would make generators free or pay the player to buy them.
 */
inline double generatorCostMultiplier(int discountLevel) {
    if (discountLevel < 0) discountLevel = 0;
    double mult = 1.0 - (discountLevel * 0.05);
    if (mult < 0.05) mult = 0.05;
    return mult;
}

/*
 * maxOfflineSeconds() - Offline earnings cap: 24 hours, plus 2 hours per perk.
 */
inline double maxOfflineSeconds(int perkOfflineCap) {
    if (perkOfflineCap < 0) perkOfflineCap = 0;
    const double BASE_OFFLINE_HOURS = 24.0;
    return (BASE_OFFLINE_HOURS + (perkOfflineCap * 2.0)) * 3600.0;
}

/*
 * cappedOfflineSeconds() - Clamp an elapsed span to the offline cap.
 *
 * A negative span means the save is stamped in the future (clock change, or a
 * hand-edited timestamp) and must yield zero rather than negative earnings.
 */
inline double cappedOfflineSeconds(double elapsedSeconds, int perkOfflineCap) {
    if (!std::isfinite(elapsedSeconds) || elapsedSeconds <= 0.0) return 0.0;
    const double cap = maxOfflineSeconds(perkOfflineCap);
    return elapsedSeconds > cap ? cap : elapsedSeconds;
}

// ============================================================================
// PRESTIGE
// ============================================================================

/*
 * prestigeRequirement() - Lifetime earnings needed for the next prestige.
 *
 * Doubles each time: $100K, $200K, $400K, ...
 */
inline double prestigeRequirement(int prestigeLevel) {
    if (prestigeLevel < 0) prestigeLevel = 0;
    return 100000.0 * std::pow(2.0, prestigeLevel);
}

/*
 * canPrestige() - Whether lifetime earnings have reached the requirement.
 */
inline bool canPrestige(double lifetimeEarnings, int prestigeLevel) {
    return lifetimeEarnings >= prestigeRequirement(prestigeLevel);
}

/*
 * prestigeStars() - Spendable stars granted by prestiging.
 *
 * 5 base, +1 per prestige level, +1 per million in total lifetime earnings.
 */
inline int prestigeStars(int prestigeLevel, double totalLifetimeEarnings) {
    if (prestigeLevel < 0) prestigeLevel = 0;

    const int baseStars = 5;
    const int levelBonus = prestigeLevel;

    int earningsBonus = 0;
    if (std::isfinite(totalLifetimeEarnings) && totalLifetimeEarnings > 0.0) {
        const double millions = totalLifetimeEarnings / 1000000.0;
        // Saturate rather than overflow int on an absurd balance.
        earningsBonus = (millions >= (double)INT_MAX) ? INT_MAX : (int)millions;
    }

    // Guard the sum itself against overflow.
    if (earningsBonus > INT_MAX - baseStars - levelBonus) {
        return INT_MAX;
    }
    return baseStars + levelBonus + earningsBonus;
}

// ============================================================================
// GENERATORS
// ============================================================================

/*
 * totalGeneratorsOwned() - Sum of owned counts across all generators.
 */
inline int totalGeneratorsOwned(const std::vector<Generator>& generators) {
    int total = 0;
    for (std::size_t i = 0; i < generators.size(); ++i) {
        total += generators[i].getOwned();
    }
    return total;
}

/*
 * generatorSynergyMultiplier() - Output bonus for one generator.
 *
 * Two stacking effects:
 *   1. Ownership thresholds for the generator itself (10/25/50/100).
 *   2. Every higher-tier generator adds 1% per unit owned.
 */
inline double generatorSynergyMultiplier(const std::vector<Generator>& generators,
                                         int genIndex) {
    double multiplier = 1.0;

    if (genIndex < 0 || genIndex >= (int)generators.size()) return multiplier;

    const int owned = generators[genIndex].getOwned();

    // Threshold bonuses for this generator
    if (owned >= 10)  multiplier *= 1.1;   // +10% at 10 owned
    if (owned >= 25)  multiplier *= 1.15;  // +15% more at 25
    if (owned >= 50)  multiplier *= 1.2;   // +20% more at 50
    if (owned >= 100) multiplier *= 1.25;  // +25% more at 100

    // Higher-tier generators boost this one (1% per owned)
    for (int i = genIndex + 1; i < (int)generators.size(); ++i) {
        multiplier *= (1.0 + generators[i].getOwned() * 0.01);
    }

    return multiplier;
}

/*
 * specializationMultiplier() - Bonus for the player's chosen specialization.
 *
 * specializationId: 0 = none, 1 = Wheel, 2 = Slots, 3 = Blackjack, 4 = Roulette
 */
inline double specializationMultiplier(int specializationId) {
    if (specializationId == 0) return 1.0;
    return 1.5;  // +50% to specialized game type
}

/*
 * PassiveIncomeInputs - The non-generator factors scaling passive income.
 *
 * Grouped into a struct because totalPassiveIncome() would otherwise take six
 * positional doubles, which is easy to transpose at a call site.
 */
struct PassiveIncomeInputs {
    double prestigeMultiplier;
    double achievementMultiplier;
    int perkPassiveBoost;

    PassiveIncomeInputs()
        : prestigeMultiplier(1.0), achievementMultiplier(1.0), perkPassiveBoost(0) {}
};

/*
 * totalPassiveIncome() - Money per second from all generators.
 *
 * Generator income with per-generator synergy, then scaled by the Income Boost
 * upgrade, the prestige multiplier, the achievement multiplier, and the passive
 * boost perk.
 */
inline double totalPassiveIncome(const std::vector<Generator>& generators,
                                const std::vector<Upgrade>& upgrades,
                                const PassiveIncomeInputs& in) {
    double total = 0.0;

    for (std::size_t i = 0; i < generators.size(); ++i) {
        const double baseIncome = generators[i].getIncomePerSecond();
        total += baseIncome * generatorSynergyMultiplier(generators, (int)i);
    }

    total *= incomeMultiplier(upgrades);
    total *= in.prestigeMultiplier;
    total *= in.achievementMultiplier;
    total *= passiveBoostFromPerk(in.perkPassiveBoost);

    return total;
}

// ============================================================================
// BULK PURCHASING
// ============================================================================

/*
 * bulkCost() - Total price of buying `amount` more of a generator.
 *
 * Unit prices form a geometric series, so the sum is closed-form:
 *
 *     cost * (m^amount - 1) / (m - 1)
 *
 * A multiplier of exactly 1.0 makes that formula divide by zero, so flat
 * pricing is handled separately.
 */
inline double bulkCost(const Generator& gen, int amount, double discountMultiplier) {
    if (amount <= 0) return 0.0;

    const double currentCost = gen.getCurrentCost();
    const double multiplier = gen.getCostMultiplier();

    if (!std::isfinite(currentCost) || currentCost <= 0.0) return 0.0;

    if (!std::isfinite(multiplier) || multiplier <= 1.0) {
        return currentCost * amount * discountMultiplier;
    }

    const double series =
        currentCost * (std::pow(multiplier, amount) - 1.0) / (multiplier - 1.0);

    return series * discountMultiplier;
}

/*
 * maxAffordable() - Largest `amount` whose bulkCost() fits in availableMoney.
 *
 * Inverts the geometric series to get a starting estimate, then corrects by at
 * most a step or two for floating point error. The discount is divided out of
 * the budget first so the closed form stays exact; otherwise the correction
 * loop had to walk the entire difference by hand.
 *
 * Guarantees, which the unit tests assert directly:
 *     bulkCost(n)     <= availableMoney
 *     bulkCost(n + 1)  > availableMoney
 */
inline int maxAffordable(const Generator& gen, double availableMoney,
                         double discountMultiplier) {
    if (!std::isfinite(availableMoney) || availableMoney <= 0.0) return 0;
    if (!std::isfinite(discountMultiplier) || discountMultiplier <= 0.0) return 0;

    const double currentCost = gen.getCurrentCost();
    const double multiplier = gen.getCostMultiplier();
    if (!std::isfinite(currentCost) || currentCost <= 0.0) return 0;

    const int owned = gen.getOwned();
    if (owned >= Generator::MAX_OWNED) return 0;
    const int maxPossible = Generator::MAX_OWNED - owned;

    // Work against a pre-discount budget so the closed form below is exact.
    const double budget = availableMoney / discountMultiplier;
    if (!std::isfinite(budget) || budget <= 0.0) return 0;

    int n;
    if (!std::isfinite(multiplier) || multiplier <= 1.0) {
        const double flat = std::floor(budget / currentCost);
        n = (flat >= (double)maxPossible) ? maxPossible : (int)flat;
    } else {
        const double a = budget * (multiplier - 1.0) / currentCost + 1.0;
        if (!std::isfinite(a) || a <= 1.0) return 0;

        const double nExact = std::log(a) / std::log(multiplier);
        if (!std::isfinite(nExact)) return 0;

        n = (nExact >= (double)maxPossible) ? maxPossible : (int)std::floor(nExact);
    }

    if (n < 0) n = 0;
    if (n > maxPossible) n = maxPossible;

    // Correct for floating point error at the boundary.
    while (n > 0 && bulkCost(gen, n, discountMultiplier) > availableMoney) {
        --n;
    }
    while (n < maxPossible &&
           bulkCost(gen, n + 1, discountMultiplier) <= availableMoney) {
        ++n;
    }

    return n;
}

} // namespace economy

#endif // CORE_ECONOMY_H
