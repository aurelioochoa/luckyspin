/*
 * upgrade.h - Purchasable improvements to game mechanics
 *
 * Upgrades differ from generators in that they modify game rules rather than
 * producing passive income. Examples:
 *   - Increase win chance on wheel spin (50% -> 55% -> 60%)
 *   - Increase payout multipliers (2x -> 2.5x -> 3x)
 *   - Boost passive income from all generators
 *
 * Moved out of gambling_game.cpp so that economy calculations operating on
 * vector<Upgrade> can be unit tested without pulling in the terminal UI.
 */

#ifndef CORE_UPGRADE_H
#define CORE_UPGRADE_H

#include <string>
#include <cmath>

/*
 * UpgradeType - Enum class to categorize different upgrade types
 *
 * Enum classes (C++11) are type-safe enumerations. Unlike plain enums,
 * they don't implicitly convert to integers and their values are scoped.
 *
 * Usage: UpgradeType::WHEEL_CHANCE instead of just WHEEL_CHANCE
 */
enum class UpgradeType {
    WHEEL_CHANCE,      // Increases wheel spin win chance
    WHEEL_PAYOUT,      // Increases wheel spin payout multiplier
    SLOTS_CHANCE,      // Increases slots win chance (via weighted symbols)
    SLOTS_PAYOUT,      // Increases slots payout multiplier
    INCOME_BOOST,      // Multiplies all passive income
    LUCKY_BONUS,       // Chance for bonus multiplier on any win
    // Blackjack upgrades
    BLACKJACK_PAYOUT,  // Increases blackjack win payout
    DEALER_BUST,       // Increases chance dealer busts
    // Roulette upgrades
    ROULETTE_PAYOUT,   // Increases roulette payout multiplier
    LUCKY_NUMBER,      // Bonus payout on specific numbers
    // Plinko upgrades
    PLINKO_PAYOUT,     // Multiplies every Plinko bucket payout
    PLINKO_EDGE        // Boosts the Plinko edge (jackpot) buckets
};

class Upgrade {
private:
    std::string name;        // Display name
    std::string description; // What this upgrade does
    UpgradeType type;        // Category of upgrade
    int currentLevel;        // Current upgrade level (0 = not purchased)
    int maxLevel;            // Maximum level this upgrade can reach
    double baseCost;         // Cost for level 1
    double costMultiplier;   // Cost increase per level
    double baseValue;        // Starting value (e.g., 50% for wheel chance)
    double valuePerLevel;    // How much the value increases per level

public:
    /*
     * Constructor - Initialize all upgrade properties
     *
     * Parameters:
     *   - n: Display name
     *   - desc: Description of what the upgrade does
     *   - t: Type of upgrade (from UpgradeType enum)
     *   - maxLvl: Maximum level achievable
     *   - cost: Base cost for first level
     *   - costMult: How much cost increases per level
     *   - baseVal: Starting value before any upgrades
     *   - valPerLvl: How much value increases per level
     */
    Upgrade(const std::string& n, const std::string& desc, UpgradeType t,
            int maxLvl, double cost, double costMult,
            double baseVal, double valPerLvl)
        : name(n), description(desc), type(t), currentLevel(0), maxLevel(maxLvl),
          baseCost(cost), costMultiplier(costMult),
          baseValue(baseVal), valuePerLevel(valPerLvl) {}

    // ========================================================================
    // GETTER METHODS
    // ========================================================================

    std::string getName() const { return name; }
    std::string getDescription() const { return description; }
    UpgradeType getType() const { return type; }
    int getCurrentLevel() const { return currentLevel; }
    int getMaxLevel() const { return maxLevel; }

    /*
     * isMaxed() - Check if upgrade is at maximum level
     */
    bool isMaxed() const {
        return currentLevel >= maxLevel;
    }

    /*
     * getCurrentCost() - Get cost for the next level
     *
     * Formula: baseCost * (costMultiplier ^ currentLevel)
     * Returns 0 if already maxed (can't purchase)
     */
    double getCurrentCost() const {
        if (isMaxed()) return 0;
        return baseCost * std::pow(costMultiplier, currentLevel);
    }

    /*
     * getCurrentValue() - Get the current effect value
     *
     * Formula: baseValue + (valuePerLevel * currentLevel)
     *
     * Example for wheel chance:
     *   baseValue = 50 (50% base chance), valuePerLevel = 2
     *   Level 0: 50%   Level 1: 52%   Level 2: 54%
     */
    double getCurrentValue() const {
        return baseValue + (valuePerLevel * currentLevel);
    }

    /*
     * getNextValue() - Preview what the value will be after upgrading
     */
    double getNextValue() const {
        if (isMaxed()) return getCurrentValue();
        return baseValue + (valuePerLevel * (currentLevel + 1));
    }

    // ========================================================================
    // MODIFIER METHODS
    // ========================================================================

    /*
     * purchase() - Upgrade to the next level
     *
     * Returns: true if upgrade was successful, false if already maxed
     */
    bool purchase() {
        if (isMaxed()) return false;
        currentLevel++;
        return true;
    }

    /*
     * setLevel() - Set the upgrade level directly (for loading saved games)
     *
     * Clamps the value to valid range [0, maxLevel].
     */
    void setLevel(int level) {
        if (level < 0) level = 0;
        if (level > maxLevel) level = maxLevel;
        currentLevel = level;
    }

    /*
     * reset() - Reset upgrade to initial state (for game restart)
     */
    void reset() {
        currentLevel = 0;
    }
};

#endif // CORE_UPGRADE_H
