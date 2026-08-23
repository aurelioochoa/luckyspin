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
// CONSTRUCTION + GENERATOR/UPGRADE DEFINITIONS
// ==========================================================================

// ========================================================================
// CONSTRUCTOR
// ========================================================================
/*
 * Constructor - Special method called when an object is created
 * 
 * The syntax after the colon is called an "initialization list".
 * It's more efficient than assigning values in the constructor body
 * because members are initialized directly rather than default-initialized
 * then assigned.
 * 
 * Format: ClassName() : member1(value1), member2(value2) { body }
 */
GamblingGame::GamblingGame() : money(100.0), betAmount(10.0), round(1), 
                 totalWins(0), totalLosses(0), peakMoney(100.0),
                 lifetimeEarnings(0.0), lastSaveTime(0), gameStartTime(0),
                 prestigeLevel(0), prestigeMultiplier(1.0), totalLifetimeEarnings(0.0),
                 animationFrame(0), menuSelection(0),
                 achFirstWin(false), achHighRoller(false), achMillionaire(false),
                 achLuckyStreak(false), achGenerator10(false), achAllUpgrades(false),
                 achPrestige1(false), achPrestige5(false),
                 achPlinkoJackpot(false), currentWinStreak(0),
                 achievementMultiplier(1.0), prestigeStars(0),
                 perkStartingMoney(0), perkGeneratorDiscount(0), perkOfflineCap(0), perkPassiveBoost(0),
                 specializationId(0), interestUnlocked(false), interestRate(0.001),
                 activeEventId(0), eventEndTime(0), nextEventTime(0),
                 bjPhase(BlackjackPhase::Idle), bjBet(0.0), headlessMode(false) {
    // Initialize milestones as locked
    for (int i = 0; i < 10; i++) {
        milestoneUnlocked[i] = false;
    }
    /*
     * Randomness comes from Rng::global() (see core/rng.h), which seeds
     * itself from std::random_device mixed with the clock. No srand() call
     * is needed, and tests can use their own seeded Rng instance.
     */
    shoe.shuffle(Rng::global());
    
    // Initialize the last update time to now
    lastUpdateTime = time(0);
    gameStartTime = time(0);

    // First random event fires 3-8 minutes into the session
    nextEventTime = time(0) + 180 + Rng::global().range(0, 300);
    
    // Initialize all generators
    // Each generator has: name, shortName, baseIncome, baseCost, costMultiplier
    initializeGenerators();
    
    // Initialize all upgrades (Sprint 2)
    initializeUpgrades();
}

/*
 * initializeGenerators() - Set up all passive income generators
 * 
 * This creates the progression of generators available in the game.
 * Each generator is more expensive but produces more income.
 * 
 * The vector::push_back() method adds an element to the end of the vector.
 * We're creating Generator objects inline and adding them to our vector.
 */
void GamblingGame::initializeGenerators() {
    // Early game generators (cheap, low income)
    // Format: name, shortName, description, icon, baseIncome, baseCost, costMultiplier
    generators.push_back(Generator(
        "Tip Jar", "TIP",
        "A humble jar collecting spare change from grateful winners",
        "🫙", 0.1, 10, 1.15));
    generators.push_back(Generator(
        "Penny Slots", "PNY",
        "Old-school one-armed bandits that never stop spinning",
        "🎰", 0.5, 50, 1.15));
    generators.push_back(Generator(
        "Auto-Wheel", "AWL",
        "A mechanical wheel that spins itself around the clock",
        "\U0001F3B1", 2.0, 200, 1.18));
    
    // Mid game generators
    generators.push_back(Generator(
        "Card Shark", "CRD",
        "A skilled dealer who always finds an edge at the tables",
        "\U0001F0CF", 5.0, 500, 1.17));
    generators.push_back(Generator(
        "VIP Table", "VIP",
        "An exclusive high-stakes table for the elite gamblers",
        "\U0001F451", 18.0, 1500, 1.18));
    generators.push_back(Generator(
        "High Roller", "HRL",
        "A wealthy patron who bets big and tips even bigger",
        "\U0001F48E", 60.0, 5000, 1.19));
    
    // Late game generators
    generators.push_back(Generator(
        "Casino Floor", "FLR",
        "An entire floor of games generating constant revenue",
        "\U0001F3DB\uFE0F", 280.0, 20000, 1.20));
    generators.push_back(Generator(
        "Casino Empire", "EMP",
        "A chain of casinos spanning the globe, printing money 24/7",
        "\U0001F30D", 1600.0, 100000, 1.21));
    generators.push_back(Generator(
        "Casino Chain", "CHN",
        "A franchise operation with a casino in every major city",
        "\U0001F3E2", 10000.0, 600000, 1.22));
    generators.push_back(Generator(
        "Offshore Casino", "OFS",
        "A floating palace beyond the reach of any gaming commission",
        "\U0001F6A2", 60000.0, 4000000, 1.23));
    generators.push_back(Generator(
        "Money Printer", "PRT",
        "Technically legal. Nobody asks questions at this level",
        "\U0001F5A8\uFE0F", 400000.0, 25000000, 1.24));
}

/*
 * initializeUpgrades() - Set up all available upgrades (Sprint 2)
 * 
 * Each upgrade has:
 *   - name: Display name
 *   - description: What it does
 *   - type: UpgradeType enum value
 *   - maxLevel: Maximum upgrade level
 *   - baseCost: Cost for first level
 *   - costMultiplier: How much cost increases per level
 *   - baseValue: Starting value (before any upgrades)
 *   - valuePerLevel: How much value improves per level
 */
void GamblingGame::initializeUpgrades() {
    // ═══════════════════════════════════════════════════════════════════
    // WHEEL SPIN UPGRADES
    // Base: 50% chance, 2x payout
    // ═══════════════════════════════════════════════════════════════════
    upgrades.push_back(Upgrade(
        "Lucky Wheel",
        "Grease the wheel bearings for better odds. Each level adds +2% win chance.",
        UpgradeType::WHEEL_CHANCE,
        5, 100, 2.5, 50.0, 2.0
    ));
    
    upgrades.push_back(Upgrade(
        "Wheel Jackpot",
        "Upgrade the prize pool! Each level adds +0.25x to your payout multiplier.",
        UpgradeType::WHEEL_PAYOUT,
        4, 200, 3.0, 2.0, 0.25
    ));
    
    // ═══════════════════════════════════════════════════════════════════
    // SLOTS UPGRADES
    // Base: ~16.7% chance (1/6), 5x payout
    // ═══════════════════════════════════════════════════════════════════
    upgrades.push_back(Upgrade(
        "Loaded Reels",
        "Weight the slot reels in your favor. Each level adds +3% match chance.",
        UpgradeType::SLOTS_CHANCE,
        5, 300, 2.5, 16.67, 3.0
    ));
    
    upgrades.push_back(Upgrade(
        "Slots Bonanza",
        "Bigger jackpots await! Each level adds +1x to your slots payout.",
        UpgradeType::SLOTS_PAYOUT,
        5, 500, 3.0, 5.0, 1.0
    ));
    
    // ═══════════════════════════════════════════════════════════════════
    // BLACKJACK UPGRADES (Sprint 5)
    // Base: 2x payout, dealer follows standard rules
    // ═══════════════════════════════════════════════════════════════════
    upgrades.push_back(Upgrade(
        "Card Counter",
        "Learn to count cards like a pro. Each level adds +0.2x to blackjack payouts.",
        UpgradeType::BLACKJACK_PAYOUT,
        5, 400, 2.5, 2.0, 0.2
    ));
    
    upgrades.push_back(Upgrade(
        "Dealer Jinx",
        "Bad luck follows the dealer. Each level adds +3% chance the dealer busts.",
        UpgradeType::DEALER_BUST,
        5, 600, 3.0, 0.0, 3.0
    ));
    
    // ═══════════════════════════════════════════════════════════════════
    // ROULETTE UPGRADES (Sprint 5)
    // Base: 2x color payout, 35x number payout
    // ═══════════════════════════════════════════════════════════════════
    upgrades.push_back(Upgrade(
        "Croupier's Friend",
        "The croupier owes you a favor. Each level adds +0.5x to roulette payouts.",
        UpgradeType::ROULETTE_PAYOUT,
        4, 350, 2.8, 2.0, 0.5
    ));
    
    upgrades.push_back(Upgrade(
        "Lucky 7s",
        "Seven is your lucky number! Each level adds +5x bonus when hitting 7, 17, or 27.",
        UpgradeType::LUCKY_NUMBER,
        5, 800, 3.2, 0.0, 5.0
    ));
    
    // ═══════════════════════════════════════════════════════════════════
    // PLINKO UPGRADES
    // Base: risk-level multiplier tables (see core/plinko.h)
    // ═══════════════════════════════════════════════════════════════════
    upgrades.push_back(Upgrade(
        "Golden Pegs",
        "Gold-plated pegs polish every payout. Each level adds +0.1x to all bucket multipliers.",
        UpgradeType::PLINKO_PAYOUT,
        5, 700, 2.8, 1.0, 0.1
    ));
    
    upgrades.push_back(Upgrade(
        "Magnet Bumpers",
        "The jackpot buckets attract bigger prizes. Each level adds +25% to edge bucket payouts.",
        UpgradeType::PLINKO_EDGE,
        4, 900, 3.0, 1.0, 0.25
    ));
    
    // ═══════════════════════════════════════════════════════════════════
    // GLOBAL UPGRADES
    // Affect all games and income
    // ═══════════════════════════════════════════════════════════════════
    upgrades.push_back(Upgrade(
        "Income Boost",
        "Optimize your casino operations. Each level adds +10% to all passive income.",
        UpgradeType::INCOME_BOOST,
        10, 250, 2.0, 100.0, 10.0
    ));
    
    upgrades.push_back(Upgrade(
        "Lucky Charm",
        "Carry a rabbit's foot for luck. Each level adds +5% chance for 2x bonus on wins.",
        UpgradeType::LUCKY_BONUS,
        5, 1000, 3.5, 0.0, 5.0
    ));
}
