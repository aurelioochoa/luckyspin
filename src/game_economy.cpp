#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <iomanip>
#include <cstring>
#include <climits>
#include <vector>
#include <sys/select.h>
#include <sstream>
#include <fstream>
#include <stdexcept>

#include "gambling_game.h"
#include "colors.h"
#include "symbols.h"
#include "utils.h"
#include "core/text_width.h"
#include "core/money.h"
#include "core/rng.h"
#include "core/economy.h"
#include "core/banner.h"
#include "core/checksum.h"

// ==========================================================================
// ECONOMY GLUE (delegates to core/economy.h)
// ==========================================================================

// ========================================================================
// UPGRADE HELPER METHODS
// ========================================================================

/*
 * getUpgradeValue() - Get the current value of a specific upgrade type
 * 
 * This method searches through all upgrades to find one matching the
 * given type and returns its current value.
 * 
 * Parameters:
 *   - type: The UpgradeType to look for
 * 
 * Returns: The current value of that upgrade, or a default if not found
 */
double GamblingGame::getUpgradeValue(UpgradeType type) const {
    return economy::upgradeValue(upgrades, type);
}

/*
 * getIncomeMultiplier() - Calculate the income boost multiplier
 * 
 * Converts the INCOME_BOOST percentage to a multiplier.
 * 100% = 1.0x, 150% = 1.5x, 200% = 2.0x, etc.
 */
double GamblingGame::getIncomeMultiplier() const {
    return economy::incomeMultiplier(upgrades);
}

/*
 * getTotalPassiveIncome() - Calculate total passive income per second
 * 
 * Iterates through all generators and sums up their income.
 * Then applies the Income Boost upgrade multiplier (Sprint 2).
 * 
 * Returns: Total income per second from all generators (with multiplier)
 */
/*
 * getAchievementCount() - Count unlocked achievements
 */
int GamblingGame::getAchievementCount() const {
    int count = 0;
    if (achFirstWin) count++;
    if (achHighRoller) count++;
    if (achMillionaire) count++;
    if (achLuckyStreak) count++;
    if (achGenerator10) count++;
    if (achAllUpgrades) count++;
    if (achPrestige1) count++;
    if (achPrestige5) count++;
    if (achPlinkoJackpot) count++;
    return count;
}

/*
 * getPassiveBoostFromPerks() - Calculate passive income multiplier from prestige perks
 */
double GamblingGame::getPassiveBoostFromPerks() const {
    return economy::passiveBoostFromPerk(perkPassiveBoost);
}

/*
 * getTotalGeneratorsOwned() - Count all generators owned (Phase C)
 */
int GamblingGame::getTotalGeneratorsOwned() const {
    return economy::totalGeneratorsOwned(generators);
}

/*
 * getGeneratorSynergyMultiplier() - Calculate synergy bonuses (Phase C)
 * 
 * Higher-tier generators boost lower tiers.
 * Threshold bonuses at 10/25/50/100 owned.
 */
double GamblingGame::getGeneratorSynergyMultiplier(int genIndex) const {
    return economy::generatorSynergyMultiplier(generators, genIndex);
}

/*
 * getSpecializationMultiplier() - Get bonus from specialization (Phase C)
 */
double GamblingGame::getSpecializationMultiplier() const {
    return economy::specializationMultiplier(specializationId);
}

double GamblingGame::getTotalPassiveIncome() const {
    economy::PassiveIncomeInputs in;
    in.prestigeMultiplier = prestigeMultiplier;
    in.achievementMultiplier = achievementMultiplier;
    in.perkPassiveBoost = perkPassiveBoost;
    return economy::totalPassiveIncome(generators, upgrades, in);
}

/*
 * checkMilestones() - Check and unlock milestones (Phase C)
 */
void GamblingGame::checkMilestones() {
    // Milestone 0: Own 10 total generators
    if (!milestoneUnlocked[0] && getTotalGeneratorsOwned() >= 10) {
        milestoneUnlocked[0] = true;
        showMilestone("Generator Collector", "Own 10 generators total");
    }
    
    // Milestone 1: Reach $10k passive income/sec
    if (!milestoneUnlocked[1] && getTotalPassiveIncome() >= 10.0) {
        milestoneUnlocked[1] = true;
        showMilestone("Income Milestone", "Reach $10/sec passive income");
    }
    
    // Milestone 2: First prestige - unlock interest system
    if (!milestoneUnlocked[2] && prestigeLevel >= 1) {
        milestoneUnlocked[2] = true;
        interestUnlocked = true;
        showMilestone("Banking Unlocked", "Interest system now available!");
    }
    
    // Milestone 3: Own 50 total generators
    if (!milestoneUnlocked[3] && getTotalGeneratorsOwned() >= 50) {
        milestoneUnlocked[3] = true;
        showMilestone("Generator Hoarder", "Own 50 generators total");
    }
    
    // Milestone 4: Reach $1M total lifetime earnings
    if (!milestoneUnlocked[4] && totalLifetimeEarnings >= 1000000.0) {
        milestoneUnlocked[4] = true;
        showMilestone("Millionaire Club", "$1M total lifetime earnings");
    }
    
    // Milestone 5: Reach prestige level 3
    if (!milestoneUnlocked[5] && prestigeLevel >= 3) {
        milestoneUnlocked[5] = true;
        showMilestone("Prestige Veteran", "Reach prestige level 3");
    }
}

/*
 * updatePassiveIncome() - Apply passive income based on elapsed time
 * 
 * This is the heart of the incremental game mechanic. It calculates
 * how much time has passed since the last update and adds the appropriate
 * amount of money based on passive income rate.
 * 
 * Called frequently in the game loop to keep income flowing.
 */
void GamblingGame::updatePassiveIncome() {
    // Get current time
    time_t now = time(0);

    // Calculate seconds elapsed since last update
    // difftime() returns the difference between two time_t values in seconds
    double deltaSeconds = difftime(now, lastUpdateTime);

    // Only update if at least some time has passed
    if (deltaSeconds > 0) {
        // Advance the random-event clock; a due event fires (and announces)
        // here, and expired boosts switch off before this tick is paid out.
        updateEvents(now);

        // Calculate income earned during this period.
        // Happy Hour doubles passive income while it is active (Phase D).
        double income = getTotalPassiveIncome() * deltaSeconds
                      * economy::eventIncomeMultiplier(activeEventId);
        
        // Add interest if unlocked (Phase C)
        if (interestUnlocked && money > 0) {
            double interest = money * interestRate * deltaSeconds;
            income += interest;
        }
        
        // Add to balance and lifetime earnings
        money += income;
        lifetimeEarnings += income;
        
        // Update peak money if we've reached a new high
        updatePeakMoney();
        
        // Reset the timer
        lastUpdateTime = now;
        
        // Check for milestone unlocks (Phase C)
        checkMilestones();
    }
}

/*
 * updateEvents() - Advance the random-event clock (Phase D).
 *
 * Called from updatePassiveIncome() on every income tick. Three things can
 * happen, in order:
 *   1. An active duration event (Happy Hour / Hot Streak) past its expiry
 *      switches off.
 *   2. If no event is due yet, nothing else happens.
 *   3. When an event is due, one is picked uniformly and announced:
 *      Happy Hour or Hot Streak run for EVENT_DURATION_SECONDS, while Money
 *      Rain pays out instantly instead of lingering.
 *
 * nextEventTime is re-scheduled BEFORE the event fires so a re-entrant call
 * (e.g. a game action ticking income mid-round) can never double-fire.
 */
void GamblingGame::updateEvents(time_t now) {
    // 1. Expire the active boost.
    if (activeEventId != economy::EVENT_NONE && now >= eventEndTime) {
        activeEventId = economy::EVENT_NONE;
        eventEndTime = 0;
    }

    // 2. Nothing due yet.
    if (now < nextEventTime) return;

    // Re-schedule first: the gap is measured from when the last event fired.
    nextEventTime = now + economy::EVENT_MIN_GAP_SECONDS
                  + Rng::global().range(0, economy::EVENT_GAP_SPREAD_SECONDS);

    // 3. Fire one event, uniformly chosen.
    const int eventId = 1 + Rng::global().below(economy::EVENT_COUNT);

    if (eventId == economy::EVENT_MONEY_RAIN) {
        // Instant payout: about 30 seconds of passive income, floored so it
        // still feels meaningful before the first generator exists.
        double grant = getTotalPassiveIncome() * 30.0;
        if (grant < 25.0) grant = 25.0;
        money += grant;
        lifetimeEarnings += grant;
        updatePeakMoney();

        showEvent(economy::eventName(eventId),
                  string("+") + formatMoney(grant) + " "
                      + economy::eventDescription(eventId));
        // No lingering multiplier: nothing to activate or expire.
    } else {
        activeEventId = eventId;
        eventEndTime = now + economy::EVENT_DURATION_SECONDS;

        showEvent(economy::eventName(eventId), economy::eventDescription(eventId));
    }
}

/*
 * applyHotStreak() - Scale a game payout by the active event multiplier.
 *
 * Every wager-style game routes its win amount through this before paying
 * out; with no event running it is an identity function. The Lucky Charm
 * doubling in applyLuckyBonus() stacks multiplicatively on top.
 */
double GamblingGame::applyHotStreak(double amount) {
    const double mult = economy::eventPayoutMultiplier(activeEventId);
    if (mult == 1.0) return amount;

    cout << "  " << Color::BG_MAGENTA << Color::BOLD << Color::WHITE
         << " HOT STREAK x1.5 " << Color::RESET << "\n";
    return amount * mult;
}

// ========================================================================
// GAME LOGIC METHODS
// ========================================================================

/*
 * updatePeakMoney() - Track the highest balance achieved
 * 
 * This is a simple helper method that updates peakMoney if current
 * balance exceeds it. Good practice to break logic into small functions.
 */
void GamblingGame::updatePeakMoney() {
    if (money > peakMoney) {
        peakMoney = money;
    }
}

/*
 * canAffordBet() - Check if player has enough money for current bet
 * 
 * Returns: true if player can afford the bet, false otherwise
 * 
 * This is a "guard" function - it prevents invalid game states
 */
bool GamblingGame::canAffordBet() {
    if (money < betAmount) {
        cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money for that bet!" 
             << Color::RESET << "\n";
        cout << "  Your balance: $" << fixed << setprecision(2) << money << "\n";
        cout << "  Current bet: $" << fixed << setprecision(2) << betAmount << "\n";
        cout << "\n  Press any key to continue...";
        getch();
        return false;
    }
    return true;
}

/*
 * applyLuckyBonus() - Check and apply lucky bonus multiplier (Sprint 2)
 * 
 * The Lucky Charm upgrade gives a chance for wins to be doubled.
 * 
 * Parameters:
 *   - amount: The base win amount
 * 
 * Returns: The amount (possibly doubled if lucky bonus triggers)
 */
double GamblingGame::applyLuckyBonus(double amount) {
    double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
    
    // Check if lucky bonus triggers (luckyChance is a percentage)
    if (Rng::global().chancePercent(luckyChance)) {
        cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED 
             << " LUCKY BONUS! 2x " << Color::RESET << "\n";
        return amount * 2.0;
    }
    return amount;
}

/*
 * applyOfflineEarnings() - Calculate earnings while the game was closed
 * 
 * This is a key feature of incremental games - progress continues even
 * when you're not playing! We calculate how much time passed and apply
 * passive income for that duration.
 * 
 * Parameters:
 *   - savedTimestamp: When the game was last saved
 */
void GamblingGame::applyOfflineEarnings(time_t savedTimestamp) {
    time_t now = time(0);
    double secondsOffline = difftime(now, savedTimestamp);
    
    // Cap offline earnings (base 24 hours + 2 hours per perk level).
    // This prevents exploits and also clamps a negative span, which happens
    // when the save is stamped in the future after a clock change.
    secondsOffline = economy::cappedOfflineSeconds(secondsOffline, perkOfflineCap);
    
    // Only apply if there's passive income and time has passed
    double passiveIncome = getTotalPassiveIncome();
    if (secondsOffline > 0 && passiveIncome > 0) {
        double offlineEarnings = passiveIncome * secondsOffline;
        money += offlineEarnings;
        lifetimeEarnings += offlineEarnings;
        updatePeakMoney();
        
        // Show welcome back message with earnings
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::GREEN);
        printCentered("WELCOME BACK!", 50, Color::GREEN + Color::BOLD);
        printLine('-', 50, Color::GREEN);
        
        cout << "\n  " << Color::WHITE << "You were away for: " << Color::CYAN;
        
        // Format time nicely
        int hours = (int)(secondsOffline / 3600);
        int minutes = (int)((secondsOffline - hours * 3600) / 60);
        int seconds = (int)(secondsOffline - hours * 3600 - minutes * 60);
        
        if (hours > 0) {
            cout << hours << "h " << minutes << "m " << seconds << "s";
        } else if (minutes > 0) {
            cout << minutes << "m " << seconds << "s";
        } else {
            cout << seconds << "s";
        }
        cout << Color::RESET << "\n";
        
        cout << "\n  " << Color::WHITE << "Offline earnings: " << Color::GREEN 
             << "+" << formatMoney(offlineEarnings) << Color::RESET << "\n";
        cout << "  " << Color::DIM << "(at " << fixed << setprecision(2) 
             << passiveIncome << "$/sec)" << Color::RESET << "\n";
        
        cout << "\n  " << Color::WHITE << "New balance: " << Color::GREEN 
             << formatMoney(money) << Color::RESET << "\n";
        
        cout << "\n  Press any key to continue...";
        getch();
    }
}

// ========================================================================
// PRESTIGE SYSTEM (Sprint 4 - Polish)
// ========================================================================

/*
 * getPrestigeRequirement() - Calculate money needed to prestige
 * 
 * Each prestige requires more money than the last.
 * Formula: 100,000 * (2 ^ prestigeLevel)
 */
double GamblingGame::getPrestigeRequirement() const {
    return economy::prestigeRequirement(prestigeLevel);
}

/*
 * canPrestige() - Check if player meets prestige requirements
 */
bool GamblingGame::canPrestige() const {
    return economy::canPrestige(lifetimeEarnings, prestigeLevel);
}

/*
 * calculatePrestigeStars() - Calculate stars earned from prestige (Phase B)
 * 
 * Formula: base 5 stars + 1 per prestige level + bonus from lifetime earnings
 */
int GamblingGame::calculatePrestigeStars() const {
    return economy::prestigeStars(prestigeLevel, totalLifetimeEarnings);
}

/*
 * getStartingMoneyFromPerk() - Calculate starting money based on perk level (Phase B)
 */
double GamblingGame::getStartingMoneyFromPerk() const {
    return economy::startingMoneyFromPerk(perkStartingMoney);
}

/*
 * updateAchievementMultiplier() - Recalculate achievement bonus (Phase B)
 * 
 * Each achievement gives +1% to all passive income.
 */
void GamblingGame::updateAchievementMultiplier() {
    achievementMultiplier = economy::achievementMultiplier(getAchievementCount());
}

/*
 * checkAchievements() - Check and unlock any earned achievements
 * 
 * Called after wins and purchases to check for new achievements.
 */
void GamblingGame::checkAchievements() {
    bool anyNewAchievement = false;
    
    // First Win
    if (!achFirstWin && totalWins >= 1) {
        achFirstWin = true;
        showAchievement("First Win", "Win your first game! (+1% income)");
        anyNewAchievement = true;
    }
    
    // Millionaire
    if (!achMillionaire && money >= 1000000) {
        achMillionaire = true;
        showAchievement("Millionaire", "Reach $1,000,000! (+1% income)");
        anyNewAchievement = true;
    }
    
    // Lucky Streak
    if (!achLuckyStreak && currentWinStreak >= 10) {
        achLuckyStreak = true;
        showAchievement("Lucky Streak", "Win 10 games in a row! (+1% income)");
        anyNewAchievement = true;
    }
    
    // Generator 10
    if (!achGenerator10) {
        for (const auto& gen : generators) {
            if (gen.getOwned() >= 10) {
                achGenerator10 = true;
                showAchievement("Generator Tycoon", "Own 10 of any generator! (+1% income)");
                anyNewAchievement = true;
                break;
            }
        }
    }
    
    // All Upgrades Maxed
    if (!achAllUpgrades) {
        if (economy::allUpgradesMaxed(upgrades)) {
            achAllUpgrades = true;
            showAchievement("Fully Upgraded", "Max out all upgrades! (+1% income)");
            anyNewAchievement = true;
        }
    }
    
    // Update multiplier if any new achievements were unlocked
    if (anyNewAchievement) {
        updateAchievementMultiplier();
    }
}

/*
 * checkHighRollerAchievement() - Check for high roller achievement
 * 
 * Called after a win with the win amount.
 */
void GamblingGame::checkHighRollerAchievement(double winAmount) {
    if (!achHighRoller && winAmount >= 1000) {
        achHighRoller = true;
        showAchievement("High Roller", "Win $1,000 in a single bet! (+1% income)");
        updateAchievementMultiplier();
    }
}

/*
 * checkPrestigeAchievements() - Check prestige-related achievements (Phase B)
 * 
 * Called after performing prestige.
 */
void GamblingGame::checkPrestigeAchievements() {
    bool anyNewAchievement = false;
    
    if (!achPrestige1 && prestigeLevel >= 1) {
        achPrestige1 = true;
        showAchievement("First Prestige", "Prestige for the first time! (+1% income)");
        anyNewAchievement = true;
    }
    if (!achPrestige5 && prestigeLevel >= 5) {
        achPrestige5 = true;
        showAchievement("Prestige Master", "Reach prestige level 5! (+1% income)");
        anyNewAchievement = true;
    }
    
    if (anyNewAchievement) {
        updateAchievementMultiplier();
    }
}

/*
 * recordWin() - Record a win and update streak
 */
void GamblingGame::recordWin(double winAmount) {
    totalWins++;
    currentWinStreak++;
    updatePeakMoney();
    checkHighRollerAchievement(winAmount);
    checkAchievements();
}

/*
 * recordLoss() - Record a loss and reset streak
 */
void GamblingGame::recordLoss() {
    totalLosses++;
    currentWinStreak = 0;
}

// ========================================================================
// GENERATOR SHOP (Sprint 1 - Core Incremental Feature)
// ========================================================================

/*
 * getGeneratorCostMultiplier() - Get cost reduction from prestige perks (Phase B)
 */
double GamblingGame::getGeneratorCostMultiplier() const {
    return economy::generatorCostMultiplier(perkGeneratorDiscount);
}

double GamblingGame::getGeneratorBulkCost(const Generator& gen, int amount) const {
    return economy::bulkCost(gen, amount, getGeneratorCostMultiplier());
}

int GamblingGame::getGeneratorMaxAffordable(const Generator& gen, double availableMoney) const {
    return economy::maxAffordable(gen, availableMoney, getGeneratorCostMultiplier());
}
