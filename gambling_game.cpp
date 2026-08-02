/*
 * ============================================================================
 * LUCKY SPIN CASINO - A Terminal-Based Incremental Gambling Game
 * ============================================================================
 * 
 * This program demonstrates several C++ concepts:
 * - Classes and Object-Oriented Programming (OOP)
 * - Member variables and methods
 * - Constructor initialization lists
 * - ANSI escape codes for terminal colors and formatting
 * - Linux terminal input handling (termios)
 * - Random number generation
 * - Control flow (loops, conditionals, switch statements)
 * - Standard library usage (iostream, iomanip, string)
 * - Vectors (dynamic arrays) for storing game objects
 * - Time-based calculations for passive income (incremental game mechanic)
 * 
 * Compile with: g++ -std=c++11 -o gambling_game gambling_game.cpp
 * Run with: ./gambling_game
 * 
 * ============================================================================
 */

// ============================================================================
// HEADER INCLUDES
// ============================================================================
// Each #include brings in a "header file" containing declarations of functions,
// classes, and constants that we can use in our program.

#include <iostream>   // For input/output streams (cout, cin, endl)
#include <cstdlib>    // For getenv(), rename(), remove()
#include <cmath>      // For pow() - power function
#include <ctime>      // For time() - used to seed random number generator
#include <string>     // For std::string class
#include <unistd.h>   // For usleep() - microsecond delays (POSIX/Linux)
#include <termios.h>  // For terminal I/O control (Linux-specific)
#include <iomanip>    // For setprecision(), fixed - number formatting
#include <cstring>    // For memset() - memory initialization
#include <climits>    // For INT_MAX
#include <vector>     // For std::vector - dynamic arrays
#include <sys/select.h> // For select() - non-blocking input check
#include <sstream>    // For ostringstream - string building
#include <fstream>    // For file I/O (save/load game) - Sprint 3
#include <stdexcept>  // For std::exception, std::invalid_argument

// Project headers. These are the single source of truth for colors, symbols
// and terminal helpers - do not redefine any of them in this file.
#include "colors.h"          // Color::   ANSI escape codes
#include "symbols.h"         // Symbols:: Cards:: UI::  display glyphs
#include "utils.h"           // getch(), clearScreen(), printLine(), terminal::
#include "core/text_width.h" // text::displayWidth() - column-accurate padding
#include "core/money.h"      // formatMoney()
#include "core/rng.h"        // Rng - seedable random number generator
#include "core/deck.h"       // cards::Card, cards::Shoe - real blackjack deck
#include "core/save_path.h"  // savepath::resolve() - XDG save location
#include "core/generator.h"  // Generator - passive income source
#include "core/upgrade.h"    // Upgrade, UpgradeType
#include "core/economy.h"    // economy:: - headless money math
#include "core/banner.h"     // banner::rows() - aligned marquee header
#include "core/checksum.h"   // checksum::fnv1a() - save integrity check

// Using the standard namespace so we don't have to write "std::" everywhere
// In larger projects, it's often better to use "std::" explicitly
using namespace std;

// ============================================================================
// COLORS, SYMBOLS AND UI GLYPHS
// ============================================================================
// Color::   -> src/colors.h
// Symbols:: -> src/symbols.h (slot reel faces)
// Cards::   -> src/symbols.h (card suits)
// UI::      -> src/symbols.h (box drawing, arrows, icons)
//
// They used to be duplicated here, which let the two copies drift apart.
// Add new glyphs to the headers, not to this file.

// ============================================================================
// GENERATOR AND UPGRADE CLASSES
// ============================================================================
// Generator   -> src/core/generator.h
// Upgrade     -> src/core/upgrade.h
// UpgradeType -> src/core/upgrade.h
//
// Moved into headers so that the economy calculations in src/core/economy.h
// can operate on them without depending on this file or the terminal UI.

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

// getch(), getchWithTimeout(), clearScreen(), printLine() and printCentered()
// live in src/utils.h. formatMoney() lives in src/core/money.h.

/*
 * kbhit() - Check if a key has been pressed (non-blocking)
 * 
 * This function checks if there's input available on stdin without blocking.
 * It's essential for incremental games where we need to:
 * 1. Update passive income continuously
 * 2. Still respond to user input when it happens
 * 
 * Uses select() system call which can monitor file descriptors with a timeout.
 * We set timeout to 0 for immediate (non-blocking) check.
 * 
 * Returns: true if a key is available to read, false otherwise
 */
bool kbhit() {
    // timeval structure holds seconds and microseconds
    struct timeval tv;
    tv.tv_sec = 0;   // 0 seconds
    tv.tv_usec = 0;  // 0 microseconds = immediate return
    
    // fd_set is a set of file descriptors to monitor
    fd_set fds;
    FD_ZERO(&fds);        // Clear the set
    FD_SET(0, &fds);      // Add stdin (file descriptor 0) to the set
    
    // select() monitors file descriptors for activity
    // Parameters: highest fd + 1, read fds, write fds, error fds, timeout
    // Returns: number of ready descriptors, 0 if timeout, -1 if error
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

// ============================================================================
// MAIN MENU DEFINITION
// ============================================================================
/*
 * The menu is a single table. Previously three separate things had to be kept
 * in sync by hand - the MENU_ITEMS constant, the local items[] array in
 * displayMenu(), and the char-dispatch switch in play() - and bumping the
 * constant without adding an entry made the render loop read past the array.
 *
 * Now the entry count is derived from the table with sizeof(), section headers
 * come from each entry's section field, and dispatch switches on the entry's
 * MenuAction so -Wswitch flags any unhandled action at compile time.
 */
enum class MenuAction {
    SpinWheel, PlaySlots, Blackjack, Roulette,          // GAMES
    GeneratorShop, UpgradeShop,                          // SHOP
    ChangeBet, Prestige, Specialization, Achievements,   // OPTIONS
    Save, Restart, Quit
};

struct MenuEntry {
    const char* section;      // Header printed above this entry, or nullptr
    std::string icon;
    const char* name;
    MenuAction action;
};

namespace {
const MenuEntry MENU_TABLE[] = {
    {"GAMES",   UI::WHEEL,   "Spin the Wheel",  MenuAction::SpinWheel},
    {nullptr,   UI::SLOT,    "Play Slots",      MenuAction::PlaySlots},
    {nullptr,   UI::CARD,    "Blackjack",       MenuAction::Blackjack},
    {nullptr,   UI::DICE,    "Roulette",        MenuAction::Roulette},
    {"SHOP",    UI::STORE,   "Generator Shop",  MenuAction::GeneratorShop},
    {nullptr,   UI::UP_ARROW,"Upgrade Shop",    MenuAction::UpgradeShop},
    {"OPTIONS", UI::DOLLAR,  "Change Bet",      MenuAction::ChangeBet},
    {nullptr,   UI::STAR,    "Prestige",        MenuAction::Prestige},
    {nullptr,   UI::TARGET,  "Specialization",  MenuAction::Specialization},
    {nullptr,   UI::TROPHY,  "Achievements",    MenuAction::Achievements},
    {nullptr,   UI::FLOPPY,  "Save Game",       MenuAction::Save},
    {nullptr,   UI::REFRESH, "Restart Game",    MenuAction::Restart},
    {nullptr,   UI::DOOR,    "Quit Game",       MenuAction::Quit}
};

// Derived from the table, so adding or removing an entry can never leave the
// count out of sync with the data.
const int MENU_ITEM_COUNT = (int)(sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]));
}

// ============================================================================
// GAMBLING GAME CLASS
// ============================================================================
/*
 * Classes in C++ are blueprints for creating objects. They encapsulate:
 * - Data (member variables) - the state of the object
 * - Functions (member methods) - the behavior of the object
 * 
 * Access specifiers:
 * - private: Only accessible within the class itself
 * - public: Accessible from anywhere
 * - protected: Accessible within class and derived classes
 */

class GamblingGame {
private:
    // ========================================================================
    // MEMBER VARIABLES (Private)
    // ========================================================================
    // These store the state of our game. They are private so they can only
    // be modified through our public methods, ensuring data integrity.
    
    double money;       // Player's current balance
    double betAmount;   // Current bet amount per game
    int round;          // Current round number
    int totalWins;      // Track total number of wins
    int totalLosses;    // Track total number of losses
    double peakMoney;   // Highest balance achieved (for statistics)
    
    // ========================================================================
    // INCREMENTAL GAME VARIABLES (Sprint 1)
    // ========================================================================
    /*
     * These variables support the passive income system - the core mechanic
     * that makes this an "incremental" or "idle" game.
     */
    
    vector<Generator> generators;  // All available passive income generators
    time_t lastUpdateTime;         // Last time we calculated passive income
    double lifetimeEarnings;       // Total money earned (for statistics/prestige)
    
    // ========================================================================
    // UPGRADE SYSTEM VARIABLES (Sprint 2)
    // ========================================================================
    /*
     * Upgrades modify game mechanics rather than producing passive income.
     * They improve win chances, payouts, and provide global bonuses.
     */
    
    vector<Upgrade> upgrades;      // All available upgrades
    
    // ========================================================================
    // SAVE/LOAD SYSTEM VARIABLES (Sprint 3)
    // ========================================================================
    /*
     * Persistence allows players to save their progress and continue later.
     * We also track when the game was last saved to calculate offline earnings.
     */
    
    static const string SAVE_FILENAME;  // Bare file name, no directory
    time_t lastSaveTime;            // When the game was last saved
    time_t gameStartTime;           // When this session started (for offline calc)

    /*
     * savePath() - Full path to the save file.
     *
     * Resolved once (creating ~/.local/share/luckyspin if needed) and cached.
     * Using a directory rather than the CWD means the `lucky-spin` command
     * finds the same save no matter where it is launched from.
     */
    static const string& savePath() {
        static const string path = savepath::resolve(SAVE_FILENAME);
        return path;
    }

    // Previous save, kept so a corrupt write is recoverable.
    static string backupPath() { return savePath() + ".bak"; }

    // Written first, then renamed over savePath() so a save is never partial.
    static string tempPath() { return savePath() + ".tmp"; }
    
    // ========================================================================
    // PRESTIGE SYSTEM VARIABLES (Sprint 4)
    // ========================================================================
    /*
     * Prestige allows players to reset progress in exchange for permanent bonuses.
     * This extends gameplay by giving long-term goals.
     */
    
    int prestigeLevel;              // Number of times player has prestiged
    double prestigeMultiplier;      // Bonus multiplier from prestige (1.0 + 0.1 per level)
    double totalLifetimeEarnings;   // Earnings across all prestiges (for achievements)
    
    // ========================================================================
    // ANIMATION & UI VARIABLES (Sprint 7)
    // ========================================================================
    /*
     * Variables for animated UI elements and menu navigation.
     */
    
    int animationFrame;             // Frame counter for marquee animation
    int menuSelection;              // Currently selected menu item (0-indexed)
    // The menu entry count is MENU_ITEM_COUNT, derived from MENU_TABLE above.
    
    // ========================================================================
    // ACHIEVEMENT SYSTEM VARIABLES (Sprint 4)
    // ========================================================================
    /*
     * Achievements reward players for reaching milestones.
     * Each achievement is tracked by a boolean flag.
     */
    
    bool achFirstWin;               // Win your first game
    bool achHighRoller;             // Win $1,000 in a single bet
    bool achMillionaire;            // Reach $1,000,000
    bool achLuckyStreak;            // Win 10 games in a row
    bool achGenerator10;            // Own 10 of any generator
    bool achAllUpgrades;            // Max out all upgrades
    bool achPrestige1;              // Prestige for the first time
    bool achPrestige5;              // Reach prestige level 5
    int currentWinStreak;           // Track consecutive wins for achievement
    
    // ========================================================================
    // PHASE B: ACHIEVEMENT REWARDS & PRESTIGE SHOP
    // ========================================================================
    /*
     * Achievement rewards give permanent bonuses for unlocking achievements.
     * Prestige stars are earned on prestige and spent in the prestige shop.
     */
    
    double achievementMultiplier;   // Income bonus from achievements (1.0 + 0.01 per achievement)
    int prestigeStars;              // Spendable currency earned from prestige
    
    // Prestige shop perk levels
    int perkStartingMoney;          // Start with more money after prestige
    int perkGeneratorDiscount;      // Generators cost less
    int perkOfflineCap;             // Offline earnings cap increase
    int perkPassiveBoost;           // Permanent passive income multiplier
    
    // ========================================================================
    // PHASE C: MILESTONES, SPECIALIZATION, SYNERGIES
    // ========================================================================
    /*
     * Milestones unlock new mechanics at progression thresholds.
     * Specialization gives strategic identity to playstyles.
     * Synergies add depth to generator combinations.
     */
    
    // Milestone unlocks
    bool milestoneUnlocked[10];     // Track which milestones are unlocked
    
    // Specialization system
    int specializationId;           // 0=none, 1=Wheel, 2=Slots, 3=Blackjack, 4=Roulette
    
    // Interest/bank system (unlocked by milestone)
    bool interestUnlocked;          // Whether interest system is available
    double interestRate;            // Interest rate per second
    
    // ========================================================================
    // BLACKJACK SHOE
    // ========================================================================
    /*
     * A 6-deck shoe shared across hands. Persisting it between hands is what
     * makes the deck finite: cards already dealt are gone until it reshuffles.
     */
    cards::Shoe shoe;
    
public:
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
    GamblingGame() : money(100.0), betAmount(10.0), round(1), 
                     totalWins(0), totalLosses(0), peakMoney(100.0),
                     lifetimeEarnings(0.0), lastSaveTime(0), gameStartTime(0),
                     prestigeLevel(0), prestigeMultiplier(1.0), totalLifetimeEarnings(0.0),
                     animationFrame(0), menuSelection(0),
                     achFirstWin(false), achHighRoller(false), achMillionaire(false),
                     achLuckyStreak(false), achGenerator10(false), achAllUpgrades(false),
                     achPrestige1(false), achPrestige5(false), currentWinStreak(0),
                     achievementMultiplier(1.0), prestigeStars(0),
                     perkStartingMoney(0), perkGeneratorDiscount(0), perkOfflineCap(0), perkPassiveBoost(0),
                     specializationId(0), interestUnlocked(false), interestRate(0.001) {
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
    void initializeGenerators() {
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
            "🎡", 2.0, 200, 1.18));
        
        // Mid game generators
        generators.push_back(Generator(
            "Card Shark", "CRD",
            "A skilled dealer who always finds an edge at the tables",
            "🃏", 5.0, 500, 1.20));
        generators.push_back(Generator(
            "VIP Table", "VIP",
            "An exclusive high-stakes table for the elite gamblers",
            "👑", 15.0, 1500, 1.22));
        generators.push_back(Generator(
            "High Roller", "HRL",
            "A wealthy patron who bets big and tips even bigger",
            "💎", 50.0, 5000, 1.25));
        
        // Late game generators
        generators.push_back(Generator(
            "Casino Floor", "FLR",
            "An entire floor of games generating constant revenue",
            "🏛️", 200.0, 20000, 1.28));
        generators.push_back(Generator(
            "Casino Empire", "EMP",
            "A chain of casinos spanning the globe, printing money 24/7",
            "🌍", 1000.0, 100000, 1.30));
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
    void initializeUpgrades() {
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
    double getUpgradeValue(UpgradeType type) const {
        return economy::upgradeValue(upgrades, type);
    }
    
    /*
     * getIncomeMultiplier() - Calculate the income boost multiplier
     * 
     * Converts the INCOME_BOOST percentage to a multiplier.
     * 100% = 1.0x, 150% = 1.5x, 200% = 2.0x, etc.
     */
    double getIncomeMultiplier() const {
        return economy::incomeMultiplier(upgrades);
    }
    
    // ========================================================================
    // DISPLAY METHODS
    // ========================================================================
    
    /*
     * displayHeader() - Show the game title with animated marquee (Sprint 7)
     * 
     * The stars around the title blink like a casino marquee sign.
     * Animation is based on animationFrame counter that increments each refresh.
     */
    void displayHeader() {
        // The rows are built (and width-tested) by banner::rows(). Colors are
        // injected here: borders yellow, stars cycling, title bold yellow.
        static const string starColors[] = {
            Color::YELLOW, Color::RED, Color::GREEN, Color::CYAN
        };

        banner::Colors colors;
        colors.border = Color::YELLOW;
        colors.star   = starColors[animationFrame % 4];
        colors.title  = Color::BOLD + Color::YELLOW;
        colors.reset  = Color::RESET;

        const vector<string> rows = banner::rows(animationFrame, colors);
        for (size_t i = 0; i < rows.size(); i++) {
            cout << "  " << rows[i] << "\n";
        }

        // Increment animation frame
        animationFrame++;
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
    int getAchievementCount() const {
        int count = 0;
        if (achFirstWin) count++;
        if (achHighRoller) count++;
        if (achMillionaire) count++;
        if (achLuckyStreak) count++;
        if (achGenerator10) count++;
        if (achAllUpgrades) count++;
        if (achPrestige1) count++;
        if (achPrestige5) count++;
        return count;
    }
    
    /*
     * getPassiveBoostFromPerks() - Calculate passive income multiplier from prestige perks
     */
    double getPassiveBoostFromPerks() const {
        return economy::passiveBoostFromPerk(perkPassiveBoost);
    }
    
    /*
     * getTotalGeneratorsOwned() - Count all generators owned (Phase C)
     */
    int getTotalGeneratorsOwned() const {
        return economy::totalGeneratorsOwned(generators);
    }
    
    /*
     * getGeneratorSynergyMultiplier() - Calculate synergy bonuses (Phase C)
     * 
     * Higher-tier generators boost lower tiers.
     * Threshold bonuses at 10/25/50/100 owned.
     */
    double getGeneratorSynergyMultiplier(int genIndex) const {
        return economy::generatorSynergyMultiplier(generators, genIndex);
    }
    
    /*
     * getSpecializationMultiplier() - Get bonus from specialization (Phase C)
     */
    double getSpecializationMultiplier() const {
        return economy::specializationMultiplier(specializationId);
    }
    
    double getTotalPassiveIncome() const {
        economy::PassiveIncomeInputs in;
        in.prestigeMultiplier = prestigeMultiplier;
        in.achievementMultiplier = achievementMultiplier;
        in.perkPassiveBoost = perkPassiveBoost;
        return economy::totalPassiveIncome(generators, upgrades, in);
    }
    
    /*
     * checkMilestones() - Check and unlock milestones (Phase C)
     */
    void checkMilestones() {
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
     * showMilestone() - Display milestone unlock notification (Phase C)
     */
    void showMilestone(const string& name, const string& desc) {
        cout << "\n";
        cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD 
             << " 🏆 MILESTONE UNLOCKED! 🏆 " << Color::RESET << "\n";
        cout << "  " << Color::YELLOW << name << Color::RESET << "\n";
        cout << "  " << Color::DIM << desc << Color::RESET << "\n";
        usleep(1500000);
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
    void updatePassiveIncome() {
        // Get current time
        time_t now = time(0);
        
        // Calculate seconds elapsed since last update
        // difftime() returns the difference between two time_t values in seconds
        double deltaSeconds = difftime(now, lastUpdateTime);
        
        // Only update if at least some time has passed
        if (deltaSeconds > 0) {
            // Calculate income earned during this period
            double income = getTotalPassiveIncome() * deltaSeconds;
            
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
     * displayStats() - Show current game statistics with Unicode decorations
     * 
     * Updated in Sprint 6 to use Unicode icons and show owned generators.
     */
    void displayStats() {
        cout << "\n";
        cout << "  " << UI::TL_CORNER;
        for (int i = 0; i < 46; i++) cout << UI::H_LINE;
        cout << UI::TR_CORNER << "\n";
        
        // Display balance with icon
        cout << "  " << UI::V_LINE << " " << UI::MONEY << " Balance: ";
        if (money >= 100) {
            cout << Color::GREEN;
        } else if (money >= 50) {
            cout << Color::YELLOW;
        } else {
            cout << Color::RED;
        }
        cout << formatMoney(money) << Color::RESET;
        
        // Display current bet
        cout << "  " << UI::DICE << " Bet: " << Color::CYAN 
             << "$" << fixed << setprecision(2) << betAmount << Color::RESET;
        
        // Display round number
        cout << "  " << UI::CHART << " Rnd: " << Color::MAGENTA 
             << round << Color::RESET;
        cout << " " << UI::V_LINE << "\n";
        
        // Display passive income with fire icon if high
        double passiveIncome = getTotalPassiveIncome();
        cout << "  " << UI::V_LINE << " ";
        if (passiveIncome >= 10) {
            cout << UI::FIRE;
        } else {
            cout << UI::CLOCK;
        }
        cout << " Income: " << Color::BOLD << Color::GREEN;
        if (passiveIncome > 0) {
            cout << "+$" << fixed << setprecision(2) << passiveIncome << "/sec";
        } else {
            cout << Color::DIM << "$0.00/sec";
        }
        cout << Color::RESET;
        
        // Show prestige level if > 0
        if (prestigeLevel > 0) {
            cout << "  " << UI::STAR << " P" << prestigeLevel << " (" 
                 << fixed << setprecision(1) << prestigeMultiplier << "x)";
        }
        cout << " " << UI::V_LINE << "\n";
        
        // Win/Loss statistics with icons
        cout << "  " << UI::V_LINE << " " << Color::GREEN << UI::CHECK << " Wins: " << totalWins << Color::RESET
             << "  " << Color::RED << UI::CROSS_X << " Losses: " << totalLosses << Color::RESET
             << "  " << UI::TROPHY << " Peak: " << Color::YELLOW << formatMoney(peakMoney) 
             << Color::RESET << " " << UI::V_LINE << "\n";
        
        // Show owned generators as icons
        cout << "  " << UI::V_LINE << " Generators: ";
        bool hasGenerators = false;
        for (const auto& gen : generators) {
            if (gen.getOwned() > 0) {
                hasGenerators = true;
                cout << gen.getIcon();
                if (gen.getOwned() > 1) {
                    cout << "x" << gen.getOwned() << " ";
                } else {
                    cout << " ";
                }
            }
        }
        if (!hasGenerators) {
            cout << Color::DIM << "(none - visit shop!)" << Color::RESET;
        }
        cout << UI::V_LINE << "\n";
        
        cout << "  " << UI::BL_CORNER;
        for (int i = 0; i < 46; i++) cout << UI::H_LINE;
        cout << UI::BR_CORNER << "\n";
    }
    
    /*
     * menuDesc() - Live description for a menu entry
     *
     * Descriptions that show current odds, bet size or prestige level are
     * computed here each render, so the table above only holds static text.
     */
    string menuDesc(MenuAction action) const {
        switch (action) {
            case MenuAction::SpinWheel:
                return "(" + to_string((int)getUpgradeValue(UpgradeType::WHEEL_CHANCE))
                     + "% chance, "
                     + to_string(getUpgradeValue(UpgradeType::WHEEL_PAYOUT)).substr(0, 4) + "x)";
            case MenuAction::PlaySlots:
                return "(" + to_string((int)getUpgradeValue(UpgradeType::SLOTS_CHANCE))
                     + "% chance, "
                     + to_string(getUpgradeValue(UpgradeType::SLOTS_PAYOUT)).substr(0, 3) + "x)";
            case MenuAction::Blackjack:
                return "(payout: "
                     + to_string(getUpgradeValue(UpgradeType::BLACKJACK_PAYOUT)).substr(0, 3) + "x)";
            case MenuAction::Roulette:        return "(up to 35x payout)";
            case MenuAction::GeneratorShop:   return "(passive income)";
            case MenuAction::UpgradeShop:     return "(improve odds)";
            case MenuAction::ChangeBet:
                return "(current: $" + to_string((int)betAmount) + ")";
            case MenuAction::Prestige:
                return "(level " + to_string(prestigeLevel) + ")";
            case MenuAction::Specialization:
                return "(" + specializationName() + ")";
            case MenuAction::Restart:         return "(reset all progress)";
            case MenuAction::Achievements:
            case MenuAction::Save:
            case MenuAction::Quit:            return "";
        }
        return "";
    }

    /*
     * specializationName() - Display name of the chosen specialization
     */
    string specializationName() const {
        switch (specializationId) {
            case 1: return "Wheel";
            case 2: return "Slots";
            case 3: return "Blackjack";
            case 4: return "Roulette";
            default: return "none";
        }
    }

    /*
     * displayMenu() - Show the main menu with arrow/WASD navigation (Sprint 7)
     * 
     * Menu items are highlighted based on menuSelection.
     * Use W/S or Up/Down arrows to navigate, Enter/Space to select.
     */
    void displayMenu() {
        cout << "\n";
        cout << "  " << Color::DIM << "Use " << Color::RESET << Color::CYAN << "W/S"
             << Color::RESET << Color::DIM << " or " << Color::RESET << Color::CYAN
             << "↑/↓" << Color::RESET << Color::DIM << " to navigate, "
             << Color::RESET << Color::CYAN << "Enter" << Color::RESET
             << Color::DIM << " to select" << Color::RESET << "\n\n";

        // Display menu items with cursor
        for (int i = 0; i < MENU_ITEM_COUNT; i++) {
            const MenuEntry& item = MENU_TABLE[i];

            // Section headers come from the table rather than hardcoded indices
            if (item.section != nullptr) {
                if (i > 0) cout << "\n";
                cout << "  " << Color::BOLD << item.section << Color::RESET << "\n";
            }

            const string desc = menuDesc(item.action);

            // Highlight selected item
            if (i == menuSelection) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << item.icon << " ";
                // Pad to a consistent column count (not byte count)
                printPadded(item.name, 20);
                cout << Color::RESET << " " << Color::CYAN << desc << Color::RESET << "\n";
            } else {
                cout << "    " << item.icon << " " << Color::WHITE;
                printPadded(item.name, 20);
                cout << Color::RESET;
                cout << " " << Color::DIM << desc << Color::RESET << "\n";
            }
        }
    }
    
    /*
     * displayStatus() - Main display function combining all elements
     */
    void displayStatus() {
        clearScreen();
        displayHeader();
        displayStats();
        displayMenu();
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
    void updatePeakMoney() {
        if (money > peakMoney) {
            peakMoney = money;
        }
    }
    
    /*
     * changeBet() - Allow player to modify their bet amount
     * 
     * Uses arrow/WASD navigation with preset bet amounts and custom option.
     * Demonstrates input validation - we check that the new bet is:
     * 1. Greater than 0 (can't bet nothing or negative)
     * 2. Less than or equal to current balance (can't bet more than you have)
     */
    void changeBet() {
        int betSelection = 0;
        bool inBetMenu = true;
        
        // Preset bet amounts
        double presets[] = {1, 5, 10, 25, 50, 100, 250, 500, 1000};
        int presetCount = 9;
        int totalOptions = presetCount + 2;  // presets + custom + back
        
        while (inBetMenu) {
            clearScreen();
            displayHeader();
            
            cout << "\n";
            printLine('-', 50, Color::YELLOW);
            printCentered("\xF0\x9F\x92\xB5 CHANGE BET \xF0\x9F\x92\xB5", 50, Color::YELLOW + Color::BOLD);
            printLine('-', 50, Color::YELLOW);
            
            cout << "\n  " << Color::WHITE << "Current bet: " << Color::CYAN 
                 << "$" << fixed << setprecision(2) << betAmount << Color::RESET << "\n";
            cout << "  " << Color::WHITE << "Available balance: " << Color::GREEN << "$" << fixed 
                 << setprecision(2) << money << Color::RESET << "\n";
            
            cout << "\n  " << Color::DIM << "Use W/S to navigate, Enter to select, Q to exit" << Color::RESET << "\n\n";
            
            // Display preset bet options
            for (int i = 0; i < presetCount; i++) {
                bool canAfford = (presets[i] <= money);
                bool isSelected = (betSelection == i);
                bool isCurrent = (presets[i] == betAmount);
                
                if (isSelected) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " $" << fixed << setprecision(0) << presets[i] << Color::RESET;
                } else {
                    cout << "    ";
                    if (canAfford) {
                        cout << Color::WHITE << "$" << fixed << setprecision(0) << presets[i] << Color::RESET;
                    } else {
                        cout << Color::RED << "$" << fixed << setprecision(0) << presets[i] << Color::RESET;
                    }
                }
                
                if (isCurrent) {
                    cout << " " << Color::CYAN << "(current)" << Color::RESET;
                }
                if (!canAfford) {
                    cout << " " << Color::DIM << "(insufficient funds)" << Color::RESET;
                }
                cout << "\n";
            }
            
            cout << "\n";
            
            // Custom amount option
            if (betSelection == presetCount) {
                cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD << " " << UI::ARROW_RIGHT << " Custom Amount" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::YELLOW << "Custom Amount" << Color::RESET << "\n";
            }
            
            // Back option
            if (betSelection == presetCount + 1) {
                cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Back to Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::DIM << "Back to Menu" << Color::RESET << "\n";
            }
            
            char choice = getch();
            
            // Handle arrow keys
            if (choice == 27) {
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';
                    else if (seq2 == 'B') choice = 's';
                }
                if (choice == 27) continue;
            }
            
            // Navigation
            if (choice == 'w' || choice == 'W') {
                betSelection = (betSelection - 1 + totalOptions) % totalOptions;
            } else if (choice == 's' || choice == 'S') {
                betSelection = (betSelection + 1) % totalOptions;
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (betSelection == presetCount + 1) {
                    // Back to menu
                    inBetMenu = false;
                } else if (betSelection == presetCount) {
                    // Custom amount
                    cout << "\n  Enter custom bet amount: " << Color::YELLOW << "$" << Color::RESET;
                    double newBet;
                    cin >> newBet;
                    
                    if (newBet > 0 && newBet <= money) {
                        betAmount = newBet;
                        cout << "\n  " << Color::GREEN << UI::CHECK << " Bet updated to $" << fixed 
                             << setprecision(2) << betAmount << "!" << Color::RESET << "\n";
                        cout << "\n  Press any key to continue...";
                        getch();
                        inBetMenu = false;
                    } else if (newBet <= 0) {
                        cout << "\n  " << Color::RED << UI::CROSS_X << " Bet must be greater than $0!" 
                             << Color::RESET << "\n";
                        cout << "\n  Press any key to continue...";
                        getch();
                    } else {
                        cout << "\n  " << Color::RED << UI::CROSS_X << " Insufficient funds for that bet!" 
                             << Color::RESET << "\n";
                        cout << "\n  Press any key to continue...";
                        getch();
                    }
                } else {
                    // Preset amount
                    double selectedBet = presets[betSelection];
                    if (selectedBet <= money) {
                        betAmount = selectedBet;
                        cout << "\n  " << Color::GREEN << UI::CHECK << " Bet updated to $" << fixed 
                             << setprecision(2) << betAmount << "!" << Color::RESET << "\n";
                        cout << "\n  Press any key to continue...";
                        getch();
                        inBetMenu = false;
                    } else {
                        cout << "\n  " << Color::RED << UI::CROSS_X << " Insufficient funds for that bet!" 
                             << Color::RESET << "\n";
                        cout << "\n  Press any key to continue...";
                        getch();
                    }
                }
            } else if (choice == 'q' || choice == 'Q') {
                // Quick exit
                inBetMenu = false;
            }
        }
    }
    
    /*
     * canAffordBet() - Check if player has enough money for current bet
     * 
     * Returns: true if player can afford the bet, false otherwise
     * 
     * This is a "guard" function - it prevents invalid game states
     */
    bool canAffordBet() {
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
    double applyLuckyBonus(double amount) {
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
     * spinWheel() - The wheel spinning game
     * 
     * Rules (affected by upgrades):
     * - Win chance: 50% base + WHEEL_CHANCE upgrade
     * - Payout: 2x base + WHEEL_PAYOUT upgrade
     * - Lucky Charm can double winnings
     */
    void spinWheel() {
        // Guard clause - exit early if can't afford bet
        if (!canAffordBet()) return;
        
        // Deduct bet from balance
        money -= betAmount;
        
        // Get upgrade values for this game (Sprint 2)
        double winChance = getUpgradeValue(UpgradeType::WHEEL_CHANCE);
        double payoutMult = getUpgradeValue(UpgradeType::WHEEL_PAYOUT);
        
        cout << "\n  " << Color::YELLOW << "Spinning the wheel";
        cout << Color::DIM << " (" << fixed << setprecision(0) << winChance 
             << "% chance, " << setprecision(2) << payoutMult << "x payout)" 
             << Color::RESET;
        
        // Animation loop - creates suspense with dots
        // cout.flush() ensures output is displayed immediately
        for (int i = 0; i < 5; i++) {
            cout << ".";
            cout.flush();  // Force output buffer to display
            usleep(300000); // Sleep for 300,000 microseconds (0.3 seconds)
        }
        
        // Generate random outcome using upgraded win chance.
        // chancePercent() honours fractional percentages exactly, unlike
        // `rand() % 100 < winChance` which rounded 62.5% up to 63%.
        bool won = Rng::global().chancePercent(winChance);
        
        cout << "\n\n";
        
        if (won) {
            // Calculate win amount using upgraded payout multiplier
            double winAmount = betAmount * payoutMult;
            
            // Apply Lucky Charm bonus if it triggers
            winAmount = applyLuckyBonus(winAmount);
            
            money += winAmount;
            totalWins++;
            updatePeakMoney();
            
            // Display win message with colors
            cout << "  " << Color::BG_GREEN << Color::BOLD << Color::WHITE 
                 << "  YOU WON!  " << Color::RESET << "\n";
            cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
                 << winAmount << Color::RESET << "\n";
        } else {
            totalLosses++;
            
            // Display loss message
            cout << "  " << Color::BG_RED << Color::BOLD << Color::WHITE 
                 << "  YOU LOST  " << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                 << betAmount << Color::RESET << "\n";
        }
        
        round++;
        cout << "\n  Press any key to continue...";
        getch();
    }
    
    /*
     * getColoredSymbol() - Get a slot symbol with its associated color
     * 
     * Parameters:
     *   - index: Index into the Symbols arrays (0 to COUNT-1)
     * 
     * Returns: A string containing the colored symbol
     * 
     * This demonstrates string concatenation with the + operator
     */
    string getColoredSymbol(int index) {
        return Symbols::COLORS[index] + "[" + Symbols::ALL[index] + "]" + Color::RESET;
    }
    
    /*
     * playSlots() - The slot machine game
     * 
     * Rules (affected by upgrades):
     * - Three reels with 6 different symbols each
     * - Win chance: base ~16.7% + SLOTS_CHANCE upgrade (implemented via forced match)
     * - Payout: 5x base + SLOTS_PAYOUT upgrade
     * - Lucky Charm can double winnings
     */
    void playSlots() {
        if (!canAffordBet()) return;
        
        money -= betAmount;
        
        // Get upgrade values (Sprint 2)
        double winChance = getUpgradeValue(UpgradeType::SLOTS_CHANCE);
        double payoutMult = getUpgradeValue(UpgradeType::SLOTS_PAYOUT);
        
        cout << "\n  " << Color::YELLOW << "Rolling slots...";
        cout << Color::DIM << " (" << fixed << setprecision(0) << winChance 
             << "% chance, " << setprecision(1) << payoutMult << "x payout)" 
             << Color::RESET << "\n\n";
        
        // Array to store the three slot positions
        // Each element will be an index into Symbols::ALL
        int slots[3];
        
        // Animation loop - spin the slots with gradually slowing speed
        for (int spin = 0; spin < 15; spin++) {
            // Generate random symbols for each reel
            for (int i = 0; i < 3; i++) {
                slots[i] = Rng::global().below(Symbols::COUNT);
            }
            
            // \r is carriage return - moves cursor to start of line
            // This creates the animation effect by overwriting previous output
            cout << "\r    | " << getColoredSymbol(slots[0]) 
                 << " | " << getColoredSymbol(slots[1]) 
                 << " | " << getColoredSymbol(slots[2]) 
                 << " |" << flush;
            
            // Gradually slow down the animation
            // usleep takes microseconds: 50000 = 0.05 seconds
            // As spin increases, delay increases (slowing effect)
            usleep(50000 + spin * 50000);
        }
        
        // Determine win using upgraded chance (Sprint 2)
        // Instead of relying on natural triple match (~16.7%), we use the upgrade value
        // If the random roll succeeds, we force a match; otherwise keep random result
        bool won = Rng::global().chancePercent(winChance);
        
        // If won by upgrade chance, force all slots to match
        if (won) {
            slots[1] = slots[0];
            slots[2] = slots[0];
            // Redraw the final result with matching symbols
            cout << "\r    | " << getColoredSymbol(slots[0]) 
                 << " | " << getColoredSymbol(slots[1]) 
                 << " | " << getColoredSymbol(slots[2]) 
                 << " |" << flush;
        }
        
        cout << "\n\n";
        
        // Check for win condition: all three symbols match
        if (slots[0] == slots[1] && slots[1] == slots[2]) {
            // Calculate win with upgraded payout
            double winAmount = betAmount * payoutMult;
            
            // Apply Lucky Charm bonus if it triggers
            winAmount = applyLuckyBonus(winAmount);
            
            money += winAmount;
            totalWins++;
            updatePeakMoney();
            
            // Special jackpot display
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED 
                 << "  *** JACKPOT! ***  " << Color::RESET << "\n";
            cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
                 << winAmount << Color::RESET << "\n";
        } else {
            totalLosses++;
            
            cout << "  " << Color::DIM << "No match this time." << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                 << betAmount << Color::RESET << "\n";
        }
        
        round++;
        cout << "\n  Press any key to continue...";
        getch();
    }
    
    // ========================================================================
    // NEW GAMES (Sprint 4 - Polish)
    // ========================================================================
    
    /*
     * playBlackjack() - Simplified Blackjack game
     * 
     * Rules:
     * - Player and dealer each get two cards
     * - Player can Hit (draw card) or Stand (keep current hand)
     * - Goal: Get as close to 21 as possible without going over
     * - Face cards (J, Q, K) = 10, Ace = 1 or 11
     * - Dealer must hit on 16 or less, stand on 17+
     * - Win pays 1.5x (Blackjack pays 2x)
     */
    void playBlackjack() {
        if (!canAffordBet()) return;
        
        money -= betAmount;
        
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::GREEN);
        printCentered("BLACKJACK", 50, Color::GREEN + Color::BOLD);
        printLine('-', 50, Color::GREEN);
        
        /*
         * Cards come from a real shoe (see core/deck.h): finite, shuffled, and
         * dealt without replacement, so the suit shown belongs to the card and
         * face cards keep their identity instead of all rendering as "10".
         */
        if (shoe.needsShuffle()) {
            shoe.shuffle(Rng::global());
        }
        
        auto drawCard = [&]() -> cards::Card {
            return shoe.draw(Rng::global());
        };
        
        // Render a card as [Rank+Suit], colored by its real suit
        auto displayCard = [](const cards::Card& card) -> string {
            const string suitGlyph = Cards::SUITS[card.suit];
            const string suitColor = cards::isRedSuit(card.suit) ? Color::RED : Color::WHITE;
            return "[" + suitColor + cards::rankName(card.rank) + suitGlyph + Color::RESET + "]";
        };
        
        // Deal initial cards: player, dealer, player, dealer
        vector<cards::Card> playerHand, dealerHand;
        playerHand.push_back(drawCard());
        dealerHand.push_back(drawCard());
        playerHand.push_back(drawCard());
        dealerHand.push_back(drawCard());
        
        bool playerBust = false;
        bool playerStand = false;
        
        // Player's turn with arrow/WASD navigation
        int bjSelection = 0;  // 0=Hit, 1=Stand, 2=Quit
        
        while (!playerBust && !playerStand) {
            // Clear and redraw for clean display
            clearScreen();
            displayHeader();
            cout << "\n";
            printLine('-', 50, Color::GREEN);
            printCentered(Cards::SPADE + " BLACKJACK " + Cards::HEART, 50, Color::GREEN + Color::BOLD);
            printLine('-', 50, Color::GREEN);
            
            int playerTotal = cards::handTotal(playerHand);
            
            // Display current state with card suits
            cout << "\n  " << Color::WHITE << "Dealer shows: " << Color::CYAN 
                 << displayCard(dealerHand[0]) << " [" << Color::DIM << "??" << Color::RESET << "]" 
                 << Color::RESET << "\n";
            
            cout << "  " << Color::WHITE << "Your hand:    " << Color::YELLOW;
            for (size_t i = 0; i < playerHand.size(); i++) {
                cout << displayCard(playerHand[i]) << " ";
            }
            cout << Color::RESET << "= " << Color::BOLD << playerTotal
                 << (cards::isSoft(playerHand) ? " (soft)" : "") << Color::RESET << "\n";
            
            if (playerTotal == 21) {
                cout << "\n  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                     << " " << Cards::SPADE << " BLACKJACK! " << Cards::SPADE << " " << Color::RESET << "\n";
                playerStand = true;
                usleep(1000000);
                break;
            }
            
            if (playerTotal > 21) {
                playerBust = true;
                break;
            }
            
            // Menu-style selection for Hit/Stand/Quit
            cout << "\n  " << Color::DIM << "Use W/S or " << UI::ARROW_UP << "/" << UI::ARROW_DOWN << " to navigate, Enter to select" << Color::RESET << "\n\n";
            
            // Hit option
            if (bjSelection == 0) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Hit" << Color::RESET << "      Draw another card\n";
            } else {
                cout << "    " << Color::WHITE << "Hit" << Color::RESET << "      " << Color::DIM << "Draw another card" << Color::RESET << "\n";
            }
            
            // Stand option
            if (bjSelection == 1) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Stand" << Color::RESET << "    Keep current hand\n";
            } else {
                cout << "    " << Color::WHITE << "Stand" << Color::RESET << "    " << Color::DIM << "Keep current hand" << Color::RESET << "\n";
            }
            
            // Quit option
            if (bjSelection == 2) {
                cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Quit" << Color::RESET << "     Forfeit bet and exit\n";
            } else {
                cout << "    " << Color::RED << "Quit" << Color::RESET << "     " << Color::DIM << "Forfeit bet and exit" << Color::RESET << "\n";
            }
            
            char choice = getch();
            
            // Handle arrow keys
            if (choice == 27) {
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';
                    else if (seq2 == 'B') choice = 's';
                }
                if (choice == 27) continue;
            }
            
            // Navigation
            if (choice == 'w' || choice == 'W') {
                bjSelection = (bjSelection - 1 + 3) % 3;
            } else if (choice == 's' || choice == 'S') {
                bjSelection = (bjSelection + 1) % 3;
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                // Execute selected action
                if (bjSelection == 0) {
                    // Hit
                    playerHand.push_back(drawCard());
                } else if (bjSelection == 1) {
                    // Stand
                    playerStand = true;
                } else if (bjSelection == 2) {
                    // Quit - forfeit the bet
                    cout << "\n  " << Color::YELLOW << "You left the table. Bet forfeited." << Color::RESET << "\n";
                    totalLosses++;
                    cout << "\n  Press any key to continue...";
                    getch();
                    return;
                }
            } else if (choice == 'h' || choice == 'H') {
                // Quick key for Hit
                playerHand.push_back(drawCard());
            } else if (choice == 'q' || choice == 'Q') {
                // Quick key for Quit
                cout << "\n  " << Color::YELLOW << "You left the table. Bet forfeited." << Color::RESET << "\n";
                totalLosses++;
                cout << "\n  Press any key to continue...";
                getch();
                return;
            }
        }
        
        int playerTotal = cards::handTotal(playerHand);
        
        if (playerBust) {
            cout << "\n  " << Color::BG_RED << Color::WHITE << Color::BOLD 
                 << " BUST! Over 21 " << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                 << betAmount << Color::RESET << "\n";
            totalLosses++;
        } else {
            // Dealer's turn
            cout << "\n  " << Color::WHITE << "Dealer reveals: " << Color::CYAN;
            for (size_t i = 0; i < dealerHand.size(); i++) {
                cout << displayCard(dealerHand[i]) << " ";
            }
            int dealerTotal = cards::handTotal(dealerHand);
            cout << Color::RESET << "= " << dealerTotal << "\n";
            usleep(800000);
            
            // Dealer draws until 17+
            // Apply Dealer Jinx upgrade - chance for dealer to bust early
            double dealerBustChance = getUpgradeValue(UpgradeType::DEALER_BUST);
            bool forcedBust = Rng::global().chancePercent(dealerBustChance);
            
            while (dealerTotal < 17) {
                cards::Card newCard = drawCard();
                dealerHand.push_back(newCard);
                dealerTotal = cards::handTotal(dealerHand);
                cout << "  Dealer draws: " << Color::CYAN << displayCard(newCard)
                     << Color::RESET << " = " << dealerTotal << "\n";
                usleep(800000);
            }
            
            // If Dealer Jinx triggered and dealer didn't naturally bust, deal the
            // dealer one more card to push them over 21.
            if (forcedBust && dealerTotal <= 21) {
                cards::Card bustCard = drawCard();
                dealerHand.push_back(bustCard);
                dealerTotal = cards::handTotal(dealerHand);
                cout << "  " << Color::MAGENTA << UI::BLACK_STAR << " Dealer Jinx! "
                     << UI::BLACK_STAR << Color::RESET << "\n";
                cout << "  Dealer draws: " << Color::CYAN << displayCard(bustCard)
                     << Color::RESET << " = " << dealerTotal << "\n";
                usleep(800000);
            }
            
            // Determine winner
            cout << "\n";
            bool dealerBust = dealerTotal > 21;
            bool playerWins = dealerBust || playerTotal > dealerTotal;
            bool push = !dealerBust && playerTotal == dealerTotal;
            
            if (dealerBust) {
                cout << "  " << Color::GREEN << "Dealer busts!" << Color::RESET << "\n";
            }
            
            if (push) {
                // Push - return bet
                money += betAmount;
                cout << "  " << Color::YELLOW << "Push! Bet returned." << Color::RESET << "\n";
            } else if (playerWins) {
                // Player wins - apply Card Counter upgrade for better payouts
                double basePayout = getUpgradeValue(UpgradeType::BLACKJACK_PAYOUT);
                double payout = cards::isNaturalBlackjack(playerHand) ? basePayout + 0.5 : basePayout;
                double winAmount = betAmount * payout;
                winAmount = applyLuckyBonus(winAmount);
                money += winAmount;
                totalWins++;
                updatePeakMoney();
                
                cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                     << " YOU WIN! " << Color::RESET << "\n";
                cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
                     << winAmount << Color::RESET << "\n";
            } else {
                // Dealer wins
                totalLosses++;
                cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD 
                     << " DEALER WINS " << Color::RESET << "\n";
                cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                     << betAmount << Color::RESET << "\n";
            }
        }
        
        round++;
        cout << "\n  Press any key to continue...";
        getch();
    }
    
    /*
     * playRoulette() - Roulette game with multiple bet types
     * 
     * Bet types:
     * - Red/Black: 48.6% chance, 2x payout
     * - Odd/Even: 48.6% chance, 2x payout
     * - Specific number (0-36): 2.7% chance, 35x payout
     */
    void playRoulette() {
        if (!canAffordBet()) return;
        
        // Red numbers on a standard roulette wheel
        int redNumbers[] = {1,3,5,7,9,12,14,16,18,19,21,23,25,27,30,32,34,36};
        int redCount = 18;
        
        auto isRed = [&](int num) -> bool {
            for (int i = 0; i < redCount; i++) {
                if (redNumbers[i] == num) return true;
            }
            return false;
        };
        
        // Menu-style bet selection with arrow/WASD navigation
        int rouletteSelection = 0;  // 0=Red, 1=Black, 2=Odd, 3=Even, 4=Number, 5=Cancel
        int betType = -1;
        int betNumber = -1;
        
        while (betType == -1) {
            clearScreen();
            displayHeader();
            cout << "\n";
            printLine('-', 50, Color::RED);
            printCentered("\xF0\x9F\x8E\xB2 ROULETTE \xF0\x9F\x8E\xB2", 50, Color::RED + Color::BOLD);
            printLine('-', 50, Color::RED);
            
            cout << "\n  " << Color::WHITE << "Balance: " << Color::GREEN 
                 << formatMoney(money) << Color::RESET << "\n";
            cout << "  " << Color::WHITE << "Bet: " << Color::CYAN << "$" << fixed 
                 << setprecision(2) << betAmount << Color::RESET << "\n";
            
            cout << "\n  " << Color::DIM << "Use W/S or " << UI::ARROW_UP << "/" << UI::ARROW_DOWN << " to navigate, Enter to select" << Color::RESET << "\n\n";
            
            // Menu options
            struct RouletteOption {
                string name;
                string desc;
                string color;
            };
            
            RouletteOption options[] = {
                {"Red", "2x payout", Color::RED},
                {"Black", "2x payout", Color::WHITE},
                {"Odd", "2x payout", Color::CYAN},
                {"Even", "2x payout", Color::CYAN},
                {"Number", "35x payout", Color::YELLOW},
                {"Cancel", "Return to menu", Color::DIM}
            };
            
            for (int i = 0; i < 6; i++) {
                if (i == rouletteSelection) {
                    if (i == 5) {
                        // Cancel option - red background
                        cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " " << options[i].name << Color::RESET;
                    } else {
                        cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " " << options[i].color << options[i].name << Color::RESET;
                    }
                    cout << "    " << options[i].desc << "\n";
                } else {
                    cout << "    " << options[i].color << options[i].name << Color::RESET;
                    cout << "    " << Color::DIM << options[i].desc << Color::RESET << "\n";
                }
            }
            
            char choice = getch();
            
            // Handle arrow keys
            if (choice == 27) {
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';
                    else if (seq2 == 'B') choice = 's';
                }
                if (choice == 27) continue;
            }
            
            // Navigation
            if (choice == 'w' || choice == 'W') {
                rouletteSelection = (rouletteSelection - 1 + 6) % 6;
            } else if (choice == 's' || choice == 'S') {
                rouletteSelection = (rouletteSelection + 1) % 6;
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (rouletteSelection == 5) {
                    // Cancel
                    return;
                } else if (rouletteSelection == 4) {
                    // Number bet - need to get the number
                    cout << "\n  Enter number (0-36): ";
                    cin >> betNumber;
                    if (betNumber < 0 || betNumber > 36) {
                        cout << "  " << Color::RED << "Invalid number!" << Color::RESET << "\n";
                        usleep(1000000);
                        betNumber = -1;
                        continue;
                    }
                    betType = 5;
                } else {
                    betType = rouletteSelection + 1;  // 1=Red, 2=Black, 3=Odd, 4=Even
                }
            } else if (choice == 'q' || choice == 'Q') {
                // Quick cancel
                return;
            }
        }
        
        // Deduct bet
        money -= betAmount;
        
        // Spin the wheel
        cout << "\n  " << Color::YELLOW << "Spinning the wheel";
        for (int i = 0; i < 5; i++) {
            cout << ".";
            cout.flush();
            usleep(400000);
        }
        
        // Generate result (0-36). below() is bias-free, unlike `rand() % 37`.
        int result = Rng::global().below(37);
        bool resultRed = isRed(result);
        
        cout << "\n\n  The ball lands on: ";
        if (result == 0) {
            cout << Color::BG_GREEN << Color::WHITE << Color::BOLD << " 0 " << Color::RESET;
        } else if (resultRed) {
            cout << Color::BG_RED << Color::WHITE << Color::BOLD << " " << result << " " << Color::RESET;
        } else {
            cout << Color::BG_WHITE << Color::BLACK << Color::BOLD << " " << result << " " << Color::RESET;
        }
        cout << "\n\n";
        
        // Check win - apply Croupier's Friend upgrade for better payouts
        bool won = false;
        double roulettePayout = getUpgradeValue(UpgradeType::ROULETTE_PAYOUT);
        double payout = roulettePayout;  // Base payout from upgrade
        
        // Check for Lucky 7s bonus (7, 17, 27)
        double lucky7Bonus = getUpgradeValue(UpgradeType::LUCKY_NUMBER);
        bool isLucky7 = (result == 7 || result == 17 || result == 27);
        
        switch (betType) {
            case 1: // Red
                won = (result != 0 && resultRed);
                break;
            case 2: // Black
                won = (result != 0 && !resultRed);
                break;
            case 3: // Odd
                won = (result != 0 && result % 2 == 1);
                break;
            case 4: // Even
                won = (result != 0 && result % 2 == 0);
                break;
            case 5: // Number
                won = (result == betNumber);
                payout = 35.0 + (roulettePayout - 2.0) * 5;  // Scale number payout
                break;
        }
        
        if (won) {
            double winAmount = betAmount * payout;
            
            // Apply Lucky 7s bonus if applicable
            if (isLucky7 && lucky7Bonus > 0) {
                winAmount += betAmount * lucky7Bonus;
                cout << "  " << Color::MAGENTA << "★ Lucky 7s Bonus! +" << lucky7Bonus 
                     << "x ★" << Color::RESET << "\n";
            }
            
            winAmount = applyLuckyBonus(winAmount);
            money += winAmount;
            totalWins++;
            updatePeakMoney();
            
            cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                 << " YOU WIN! " << Color::RESET << "\n";
            cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
                 << winAmount << Color::RESET << "\n";
        } else {
            totalLosses++;
            cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD 
                 << " NO WIN " << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                 << betAmount << Color::RESET << "\n";
        }
        
        round++;
        cout << "\n  Press any key to continue...";
        getch();
    }
    
    // ========================================================================
    // UPGRADE SHOP (Sprint 2 - Game Progression)
    // ========================================================================
    
    /*
     * displayUpgradeShop() - Show the upgrade purchase menu
     * 
     * Upgrades improve game mechanics like win chances and payouts.
     * Unlike generators, upgrades have a maximum level.
     * 
     * The shop displays:
     * - Upgrade name and current level
     * - Cost for next level
     * - Current and next values
     */
    void displayUpgradeShop() {
        bool inShop = true;
        int shopSelection = 0;  // 0 = back, 1+ = upgrades
        int maxSelection = upgrades.size();
        
        while (inShop) {
            updatePassiveIncome();
            
            clearScreen();
            printLine('=', 50, Color::BLUE);
            cout << Color::BOLD << Color::BLUE;
            printCentered("UPGRADE SHOP", 50);
            cout << Color::RESET;
            printLine('=', 50, Color::BLUE);
            
            cout << "\n  " << Color::WHITE << "Your Balance: " << Color::GREEN 
                 << formatMoney(money) << Color::RESET << "\n";
            cout << "  " << Color::DIM << "Use W/S to navigate, Enter to buy, Q to exit" << Color::RESET << "\n\n";
            
            printLine('-', 50, Color::DIM);
            
            // List all upgrades with cursor
            for (size_t i = 0; i < upgrades.size(); i++) {
                Upgrade& upg = upgrades[i];
                double cost = upg.getCurrentCost();
                bool maxed = upg.isMaxed();
                
                // Highlight selected item
                if ((int)i + 1 == shopSelection) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                    cout << upg.getName() << Color::RESET;
                } else {
                    cout << "    " << Color::WHITE << upg.getName() << Color::RESET;
                }
                
                cout << " Lv." << upg.getCurrentLevel() << "/" << upg.getMaxLevel();
                
                if (maxed) {
                    cout << Color::YELLOW << " [MAX]" << Color::RESET << "\n";
                } else {
                    string costColor = (money >= cost) ? Color::GREEN : Color::RED;
                    cout << "  " << costColor << formatMoney(cost) << Color::RESET << "\n";
                }
                
                cout << "      " << Color::DIM << upg.getDescription();
                cout << " (" << fixed << setprecision(1) << upg.getCurrentValue();
                if (!maxed) {
                    cout << " -> " << Color::GREEN << upg.getNextValue() << Color::RESET << Color::DIM;
                }
                cout << ")" << Color::RESET << "\n";
            }
            
            printLine('-', 50, Color::DIM);
            
            // Back option
            if (shopSelection == 0) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << "Back to Main Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::WHITE << "Back to Main Menu" << Color::RESET << "\n";
            }
            
            char choice = getchWithTimeout(1000);
            if (choice == 0) continue;
            
            // Handle arrow keys
            if (choice == 27) {
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';
                    else if (seq2 == 'B') choice = 's';
                }
                if (choice == 27) continue;
            }
            
            // Navigation
            if (choice == 'w' || choice == 'W') {
                shopSelection = (shopSelection - 1 + maxSelection + 1) % (maxSelection + 1);
            } else if (choice == 's' || choice == 'S') {
                shopSelection = (shopSelection + 1) % (maxSelection + 1);
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (shopSelection == 0) {
                    inShop = false;
                } else {
                    purchaseUpgrade(shopSelection - 1);
                }
            } else if (choice == 'q' || choice == 'Q' || choice == 'a' || choice == 'A') {
                inShop = false;
            } else if (choice >= '1' && choice <= '0' + (int)upgrades.size()) {
                purchaseUpgrade(choice - '1');
            } else if (choice == '0') {
                inShop = false;
            }
        }
    }
    
    /*
     * purchaseUpgrade() - Attempt to buy an upgrade level
     * 
     * Parameters:
     *   - index: Index of the upgrade in the upgrades vector
     */
    void purchaseUpgrade(int index) {
        if (index < 0 || index >= (int)upgrades.size()) {
            return;
        }
        
        Upgrade& upg = upgrades[index];
        
        if (upg.isMaxed()) {
            cout << "\n  " << Color::YELLOW << "Already at max level!" << Color::RESET;
            usleep(800000);
            return;
        }
        
        double cost = upg.getCurrentCost();
        
        if (money >= cost) {
            money -= cost;
            upg.purchase();
            
            cout << "\n  " << Color::GREEN << UI::CHECK << " Upgraded " << upg.getName() 
                 << " to Lv." << upg.getCurrentLevel() << "!" << Color::RESET;
            cout << "\n  " << Color::DIM << "New value: " << fixed << setprecision(1) 
                 << upg.getCurrentValue() << Color::RESET;
            
            usleep(800000);
        } else {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need " 
                 << formatMoney(cost) << Color::RESET;
            usleep(800000);
        }
    }
    
    // ========================================================================
    // SAVE/LOAD SYSTEM (Sprint 3 - Persistence)
    // ========================================================================
    
    /*
     * saveGame() - Save the current game state to a file
     * 
     * File format is a simple key=value text format, one per line.
     * This is easy to read, debug, and parse.
     * 
     * We save:
     * - Version number (for future compatibility)
     * - All player stats (money, wins, losses, etc.)
     * - Generator owned counts
     * - Upgrade levels
     * - Timestamp (for offline earnings calculation)
     * 
     * Returns: true if save was successful, false otherwise
     */
    bool saveGame() {
        /*
         * Written to a temp file first, then renamed into place. rename() is
         * atomic within a filesystem, so a crash or full disk mid-write leaves
         * the previous save intact instead of a half-written one.
         */
        const string temp = tempPath();

        // Built in memory first so the checksum below can cover every byte
        // of the body exactly as it will land on disk.
        ostringstream body;
        
        // Write save file header with version
        body << "LUCKY_SPIN_SAVE_V1" << endl;
        body << "timestamp=" << time(0) << endl;
        
        // Save player stats
        body << "money=" << fixed << setprecision(2) << money << endl;
        body << "betAmount=" << fixed << setprecision(2) << betAmount << endl;
        body << "round=" << round << endl;
        body << "totalWins=" << totalWins << endl;
        body << "totalLosses=" << totalLosses << endl;
        body << "peakMoney=" << fixed << setprecision(2) << peakMoney << endl;
        body << "lifetimeEarnings=" << fixed << setprecision(2) << lifetimeEarnings << endl;
        
        // Save generator owned counts
        // Format: gen_0=5, gen_1=3, etc.
        for (size_t i = 0; i < generators.size(); i++) {
            body << "gen_" << i << "=" << generators[i].getOwned() << endl;
        }
        
        // Save upgrade levels
        // Format: upg_0=2, upg_1=1, etc.
        for (size_t i = 0; i < upgrades.size(); i++) {
            body << "upg_" << i << "=" << upgrades[i].getCurrentLevel() << endl;
        }
        
        // Save prestige data (Sprint 4)
        body << "prestigeLevel=" << prestigeLevel << endl;
        body << "prestigeMultiplier=" << fixed << setprecision(2) << prestigeMultiplier << endl;
        body << "totalLifetimeEarnings=" << fixed << setprecision(2) << totalLifetimeEarnings << endl;
        
        // Save achievements (Sprint 4)
        body << "achFirstWin=" << achFirstWin << endl;
        body << "achHighRoller=" << achHighRoller << endl;
        body << "achMillionaire=" << achMillionaire << endl;
        body << "achLuckyStreak=" << achLuckyStreak << endl;
        body << "achGenerator10=" << achGenerator10 << endl;
        body << "achAllUpgrades=" << achAllUpgrades << endl;
        body << "achPrestige1=" << achPrestige1 << endl;
        body << "achPrestige5=" << achPrestige5 << endl;
        body << "currentWinStreak=" << currentWinStreak << endl;
        
        // Save Phase B data
        body << "achievementMultiplier=" << fixed << setprecision(4) << achievementMultiplier << endl;
        body << "prestigeStars=" << prestigeStars << endl;
        body << "perkStartingMoney=" << perkStartingMoney << endl;
        body << "perkGeneratorDiscount=" << perkGeneratorDiscount << endl;
        body << "perkOfflineCap=" << perkOfflineCap << endl;
        body << "perkPassiveBoost=" << perkPassiveBoost << endl;
        
        // Save Phase C data
        body << "specializationId=" << specializationId << endl;
        body << "interestUnlocked=" << interestUnlocked << endl;
        body << "interestRate=" << fixed << setprecision(6) << interestRate << endl;
        for (int i = 0; i < 10; i++) {
            body << "milestone_" << i << "=" << milestoneUnlocked[i] << endl;
        }
        
        // Integrity trailer: FNV-1a over every byte written above. The loader
        // verifies it and warns on mismatch, but still loads (warn-only).
        const string content = body.str();
        body << "checksum=" << checksum::toHex(checksum::fnv1a(content)) << endl;

        ofstream file(temp.c_str());
        if (!file.is_open()) {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to save game!" << Color::RESET;
            return false;
        }
        file << body.str();

        // Check for write errors (e.g. disk full) BEFORE replacing the old save.
        file.flush();
        const bool writeOk = file.good();
        file.close();
        
        if (!writeOk) {
            std::remove(temp.c_str());
            cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to write save data!" << Color::RESET;
            return false;
        }
        
        // Keep the previous save as a backup, then swap the new one in.
        if (savepath::fileExists(savePath())) {
            std::rename(savePath().c_str(), backupPath().c_str());
        }
        if (std::rename(temp.c_str(), savePath().c_str()) != 0) {
            std::remove(temp.c_str());
            cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to finalize save!" << Color::RESET;
            return false;
        }
        
        lastSaveTime = time(0);
        
        return true;
    }
    
    // ========================================================================
    // SAFE SAVE-FILE PARSING
    // ========================================================================
    /*
     * stod()/stoi() throw on malformed input. A save file is user-writable and
     * can be truncated by a crash, so a bad value must never take the process
     * down. These helpers report failure instead and leave the target alone.
     */
    
    static bool parseDouble(const string& text, double& out) {
        try {
            size_t consumed = 0;
            const double value = std::stod(text, &consumed);
            if (consumed == 0) return false;
            if (!(value == value)) return false;              // reject NaN
            if (value > 1e300 || value < -1e300) return false; // reject inf
            out = value;
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    static bool parseInt(const string& text, int& out) {
        try {
            size_t consumed = 0;
            const int value = std::stoi(text, &consumed);
            if (consumed == 0) return false;
            out = value;
            return true;
        } catch (const std::exception&) {
            return false;   // covers both invalid_argument and out_of_range
        }
    }
    
    static bool parseLong(const string& text, long& out) {
        try {
            size_t consumed = 0;
            const long value = std::stol(text, &consumed);
            if (consumed == 0) return false;
            out = value;
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    static bool parseBool(const string& text, bool& out) {
        int value = 0;
        if (!parseInt(text, value)) return false;
        out = (value != 0);
        return true;
    }
    
    // Non-negative doubles (money, earnings) clamped at 0.
    static bool parseMoney(const string& text, double& out) {
        double value = 0.0;
        if (!parseDouble(text, value)) return false;
        out = (value < 0.0) ? 0.0 : value;
        return true;
    }
    
    // Counts must be >= 0 and within a sane bound.
    static bool parseCount(const string& text, int& out, int maxValue) {
        int value = 0;
        if (!parseInt(text, value)) return false;
        if (value < 0) value = 0;
        if (value > maxValue) value = maxValue;
        out = value;
        return true;
    }
    
    /*
     * loadGame() - Load game state from a file
     * 
     * Reads the save file and restores all game state.
     * Also calculates offline earnings based on time passed.
     * 
     * Returns: true if load was successful, false otherwise
     */
    bool loadGame() {
        // Bring a pre-XDG save in the current directory forward, once.
        savepath::migrateLegacy(SAVE_FILENAME, savePath());
        
        time_t savedTimestamp = 0;
        
        // Try the real save; if it is missing or unreadable, fall back to the
        // backup written by the previous successful save.
        bool loaded = loadFrom(savePath(), savedTimestamp);
        if (!loaded) {
            loaded = loadFrom(backupPath(), savedTimestamp);
            if (loaded) {
                cout << "\n  " << Color::YELLOW << UI::CROSS_X
                     << " Save file was unreadable - restored from backup."
                     << Color::RESET << "\n";
                usleep(1500000);
            }
        }
        
        if (!loaded) {
            // No usable save. Normal for new players.
            return false;
        }
        
        // Recalculate achievement multiplier in case it wasn't saved (backward compatibility)
        updateAchievementMultiplier();
        
        // Calculate and apply offline earnings
        if (savedTimestamp > 0) {
            applyOfflineEarnings(savedTimestamp);
        }
        
        // Update the last update time to now
        lastUpdateTime = time(0);
        lastSaveTime = time(0);
        
        return true;
    }
    
    /*
     * loadFrom() - Parse one save file into the current game state.
     *
     * Every value goes through a parse helper that cannot throw. An
     * unrecognized key or an unparseable value is skipped, leaving that field
     * at its constructor default, so a partially corrupt file still restores
     * as much progress as possible instead of crashing the game.
     *
     * Returns: true if the file existed and had a valid header
     */
    bool loadFrom(const string& path, time_t& savedTimestamp) {
        ifstream file(path.c_str());

        if (!file.is_open()) {
            return false;
        }

        // Read raw bytes so the checksum can be verified over exactly what
        // saveGame() hashed, newline-for-newline.
        ostringstream raw;
        raw << file.rdbuf();
        const string content = raw.str();
        file.close();

        istringstream lines(content);
        string line;

        // Read and verify header
        if (!getline(lines, line) || line != "LUCKY_SPIN_SAVE_V1") {
            return false;  // Invalid or incompatible save file
        }

        // Verify the integrity trailer if present. Pre-checksum saves have no
        // trailer and load silently; a mismatch warns but still loads, so a
        // corrupt or hand-edited file never locks the player out.
        const string trailerTag = "\nchecksum=";
        const size_t trailerPos = content.rfind(trailerTag);
        if (trailerPos != string::npos) {
            string stored = content.substr(trailerPos + trailerTag.size());
            while (!stored.empty() &&
                   (stored[stored.size() - 1] == '\n' || stored[stored.size() - 1] == '\r')) {
                stored.erase(stored.size() - 1);
            }
            uint64_t expected = 0;
            const string hashedBody = content.substr(0, trailerPos + 1);
            if (!checksum::fromHex(stored, expected) ||
                checksum::fnv1a(hashedBody) != expected) {
                cout << "\n  " << Color::YELLOW << UI::CROSS_X
                     << " Save file checksum mismatch - it may be corrupt or edited."
                     << " Loading anyway." << Color::RESET << "\n";
                usleep(1500000);
            }
        }

        // Parse each line
        while (getline(lines, line)) {
            // Strip a trailing CR so files touched on Windows still load
            if (!line.empty() && line[line.size() - 1] == '\r') {
                line.erase(line.size() - 1);
            }
            
            // Find the '=' separator
            size_t eqPos = line.find('=');
            if (eqPos == string::npos) continue;  // Skip invalid lines
            
            // Split into key and value
            const string key = line.substr(0, eqPos);
            const string value = line.substr(eqPos + 1);
            
            if (key == "timestamp") {
                long parsed = 0;
                if (parseLong(value, parsed) && parsed > 0) {
                    savedTimestamp = static_cast<time_t>(parsed);
                }
            } else if (key == "money") {
                parseMoney(value, money);
            } else if (key == "betAmount") {
                parseMoney(value, betAmount);
            } else if (key == "round") {
                parseCount(value, round, INT_MAX - 1);
            } else if (key == "totalWins") {
                parseCount(value, totalWins, INT_MAX - 1);
            } else if (key == "totalLosses") {
                parseCount(value, totalLosses, INT_MAX - 1);
            } else if (key == "peakMoney") {
                parseMoney(value, peakMoney);
            } else if (key == "lifetimeEarnings") {
                parseMoney(value, lifetimeEarnings);
            } else if (key.compare(0, 4, "gen_") == 0) {
                // Generator data: gen_0, gen_1, etc.
                int index = -1;
                int owned = 0;
                if (parseInt(key.substr(4), index) &&
                    index >= 0 && index < (int)generators.size() &&
                    parseCount(value, owned, Generator::MAX_OWNED)) {
                    generators[index].setOwned(owned);
                }
            } else if (key.compare(0, 4, "upg_") == 0) {
                // Upgrade data: upg_0, upg_1, etc.
                int index = -1;
                int level = 0;
                if (parseInt(key.substr(4), index) &&
                    index >= 0 && index < (int)upgrades.size() &&
                    parseCount(value, level, upgrades[index].getMaxLevel())) {
                    upgrades[index].setLevel(level);
                }
            }
            // Prestige data (Sprint 4)
            else if (key == "prestigeLevel") {
                parseCount(value, prestigeLevel, 100000);
            } else if (key == "prestigeMultiplier") {
                double parsed = 1.0;
                if (parseDouble(value, parsed) && parsed >= 1.0) prestigeMultiplier = parsed;
            } else if (key == "totalLifetimeEarnings") {
                parseMoney(value, totalLifetimeEarnings);
            }
            // Achievement data (Sprint 4)
            else if (key == "achFirstWin") {
                parseBool(value, achFirstWin);
            } else if (key == "achHighRoller") {
                parseBool(value, achHighRoller);
            } else if (key == "achMillionaire") {
                parseBool(value, achMillionaire);
            } else if (key == "achLuckyStreak") {
                parseBool(value, achLuckyStreak);
            } else if (key == "achGenerator10") {
                parseBool(value, achGenerator10);
            } else if (key == "achAllUpgrades") {
                parseBool(value, achAllUpgrades);
            } else if (key == "achPrestige1") {
                parseBool(value, achPrestige1);
            } else if (key == "achPrestige5") {
                parseBool(value, achPrestige5);
            } else if (key == "currentWinStreak") {
                parseCount(value, currentWinStreak, INT_MAX - 1);
            }
            // Phase B data
            else if (key == "achievementMultiplier") {
                double parsed = 1.0;
                if (parseDouble(value, parsed) && parsed >= 1.0) achievementMultiplier = parsed;
            } else if (key == "prestigeStars") {
                parseCount(value, prestigeStars, INT_MAX - 1);
            } else if (key == "perkStartingMoney") {
                parseCount(value, perkStartingMoney, 10);
            } else if (key == "perkGeneratorDiscount") {
                parseCount(value, perkGeneratorDiscount, 10);
            } else if (key == "perkOfflineCap") {
                parseCount(value, perkOfflineCap, 5);
            } else if (key == "perkPassiveBoost") {
                parseCount(value, perkPassiveBoost, 20);
            }
            // Phase C data
            else if (key == "specializationId") {
                parseCount(value, specializationId, 4);
            } else if (key == "interestUnlocked") {
                parseBool(value, interestUnlocked);
            } else if (key == "interestRate") {
                double parsed = 0.0;
                if (parseDouble(value, parsed) && parsed >= 0.0) interestRate = parsed;
            } else if (key.compare(0, 10, "milestone_") == 0) {
                int index = -1;
                if (parseInt(key.substr(10), index) && index >= 0 && index < 10) {
                    parseBool(value, milestoneUnlocked[index]);
                }
            }
        }
        
        return true;
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
    void applyOfflineEarnings(time_t savedTimestamp) {
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
    
    /*
     * autoSave() - Automatically save the game if enough time has passed
     * 
     * Called periodically in the game loop. Saves every 30 seconds.
     */
    void autoSave() {
        const int AUTO_SAVE_INTERVAL = 30;  // seconds
        
        time_t now = time(0);
        if (difftime(now, lastSaveTime) >= AUTO_SAVE_INTERVAL) {
            saveGame();
            // Don't show message for auto-save to avoid interrupting gameplay
        }
    }
    
    /*
     * deleteSaveFile() - Delete the save file (for testing or reset)
     */
    void deleteSaveFile() {
        remove(savePath().c_str());
        remove(backupPath().c_str());
        remove(tempPath().c_str());
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
    double getPrestigeRequirement() const {
        return economy::prestigeRequirement(prestigeLevel);
    }
    
    /*
     * canPrestige() - Check if player meets prestige requirements
     */
    bool canPrestige() const {
        return economy::canPrestige(lifetimeEarnings, prestigeLevel);
    }
    
    /*
     * displayPrestigeMenu() - Show prestige information and options
     */
    void displayPrestigeMenu() {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('=', 50, Color::MAGENTA);
        printCentered("PRESTIGE", 50, Color::MAGENTA + Color::BOLD);
        printLine('=', 50, Color::MAGENTA);
        
        cout << "\n  " << Color::WHITE << "Current Prestige Level: " << Color::YELLOW 
             << prestigeLevel << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Current Multiplier: " << Color::GREEN 
             << fixed << setprecision(1) << prestigeMultiplier << "x" << Color::RESET << "\n";
        
        cout << "\n  " << Color::DIM << "Prestige resets your money, generators, and upgrades" << Color::RESET << "\n";
        cout << "  " << Color::DIM << "but gives you a permanent +10% income multiplier!" << Color::RESET << "\n";
        
        double requirement = getPrestigeRequirement();
        cout << "\n  " << Color::WHITE << "Lifetime Earnings: " << Color::CYAN 
             << formatMoney(lifetimeEarnings) << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Required to Prestige: " << Color::YELLOW 
             << formatMoney(requirement) << Color::RESET << "\n";
        
        double progress = (lifetimeEarnings / requirement) * 100.0;
        if (progress > 100.0) progress = 100.0;
        
        // Progress bar
        cout << "\n  Progress: [";
        int barWidth = 30;
        int filled = (int)(progress / 100.0 * barWidth);
        for (int i = 0; i < barWidth; i++) {
            if (i < filled) {
                cout << Color::GREEN << "█" << Color::RESET;
            } else {
                cout << Color::DIM << "░" << Color::RESET;
            }
        }
        cout << "] " << fixed << setprecision(1) << progress << "%\n";
        
        if (canPrestige()) {
            cout << "\n  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                 << " PRESTIGE AVAILABLE! " << Color::RESET << "\n";
            cout << "\n  " << Color::WHITE << "After prestige you will have:" << Color::RESET << "\n";
            cout << "    - Money: " << Color::CYAN << "$100.00" << Color::RESET << "\n";
            cout << "    - Multiplier: " << Color::GREEN << fixed << setprecision(1) 
                 << (prestigeMultiplier + 0.1) << "x" << Color::RESET << "\n";
            cout << "\n  " << Color::CYAN << "[P]" << Color::RESET << " Prestige Now\n";
        } else {
            cout << "\n  " << Color::DIM << "Keep earning to unlock prestige!" << Color::RESET << "\n";
        }
        
        cout << "  " << Color::CYAN << "[P]" << Color::RESET << " Prestige Now";
        if (!canPrestige()) cout << Color::DIM << " (locked)";
        cout << Color::RESET << "\n";
        cout << "  " << Color::CYAN << "[S]" << Color::RESET << " Prestige Shop (" 
             << Color::YELLOW << prestigeStars << " ★" << Color::RESET << ")\n";
        cout << "  " << Color::CYAN << "[0]" << Color::RESET << " Back to Main Menu\n";
        cout << "\n  Choice: ";
        
        char choice = getch();
        
        if ((choice == 'p' || choice == 'P') && canPrestige()) {
            performPrestige();
        } else if (choice == 's' || choice == 'S') {
            displayPrestigeShop();
        }
    }
    
    /*
     * displayPrestigeShop() - Show prestige shop with permanent perks (Phase B)
     */
    void displayPrestigeShop() {
        bool inShop = true;
        int shopSelection = 0;
        const int PERK_COUNT = 4;
        
        // Perk definitions: name, description, max level, cost per level
        struct PerkInfo {
            string name;
            string desc;
            int maxLevel;
            int baseCost;
            int* currentLevel;
        };
        
        PerkInfo perks[PERK_COUNT] = {
            {"Starting Money", "Start with more money after prestige", 10, 3, &perkStartingMoney},
            {"Generator Discount", "All generators cost less", 10, 5, &perkGeneratorDiscount},
            {"Offline Cap", "Increase offline earnings cap", 5, 8, &perkOfflineCap},
            {"Passive Boost", "Permanent +5% passive income per level", 20, 4, &perkPassiveBoost}
        };
        
        while (inShop) {
            clearScreen();
            printLine('=', 50, Color::YELLOW);
            cout << Color::BOLD << Color::YELLOW;
            printCentered("PRESTIGE SHOP", 50);
            cout << Color::RESET;
            printLine('=', 50, Color::YELLOW);
            
            cout << "\n  " << Color::WHITE << "Prestige Stars: " << Color::YELLOW 
                 << prestigeStars << " ★" << Color::RESET << "\n";
            cout << "  " << Color::DIM << "Use W/S to navigate, Enter to buy, Q to exit" 
                 << Color::RESET << "\n\n";
            
            printLine('-', 50, Color::DIM);
            
            // Display perks
            for (int i = 0; i < PERK_COUNT; i++) {
                PerkInfo& perk = perks[i];
                int level = *(perk.currentLevel);
                bool maxed = (level >= perk.maxLevel);
                int cost = perk.baseCost + (level * 2);  // Cost increases with level
                bool canAfford = (prestigeStars >= cost) && !maxed;
                
                // Highlight selected item
                if (i == shopSelection) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                    cout << perk.name << Color::RESET;
                } else {
                    cout << "    " << Color::WHITE << perk.name << Color::RESET;
                }
                
                cout << " [" << Color::CYAN << level << "/" << perk.maxLevel << Color::RESET << "]";
                
                if (maxed) {
                    cout << " " << Color::GREEN << "MAX" << Color::RESET;
                } else {
                    string costColor = canAfford ? Color::GREEN : Color::RED;
                    cout << " - Cost: " << costColor << cost << " ★" << Color::RESET;
                }
                cout << "\n";
                
                cout << "      " << Color::DIM << perk.desc << Color::RESET << "\n";
                
                // Show current effect
                if (level > 0) {
                    cout << "      " << Color::GREEN << "Current: ";
                    if (i == 0) cout << "+$" << (level * 50);
                    else if (i == 1) cout << "-" << (level * 5) << "% cost";
                    else if (i == 2) cout << "+" << (level * 2) << "h cap";
                    else if (i == 3) cout << "+" << (level * 5) << "% income";
                    cout << Color::RESET << "\n";
                }
            }
            
            printLine('-', 50, Color::DIM);
            
            // Back option
            if (shopSelection == PERK_COUNT) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << "Back to Prestige Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::WHITE << "Back to Prestige Menu" << Color::RESET << "\n";
            }
            
            char choice = getch();
            
            // Handle arrow keys
            if (choice == 27) {
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';
                    else if (seq2 == 'B') choice = 's';
                }
                if (choice == 27) continue;
            }
            
            // Navigation
            if (choice == 'w' || choice == 'W') {
                shopSelection = (shopSelection - 1 + PERK_COUNT + 1) % (PERK_COUNT + 1);
            } else if (choice == 's' || choice == 'S') {
                shopSelection = (shopSelection + 1) % (PERK_COUNT + 1);
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (shopSelection == PERK_COUNT) {
                    inShop = false;
                } else {
                    // Try to purchase perk
                    PerkInfo& perk = perks[shopSelection];
                    int level = *(perk.currentLevel);
                    if (level < perk.maxLevel) {
                        int cost = perk.baseCost + (level * 2);
                        if (prestigeStars >= cost) {
                            prestigeStars -= cost;
                            (*(perk.currentLevel))++;
                            cout << "\n  " << Color::GREEN << UI::CHECK << " Purchased " 
                                 << perk.name << " (Level " << (*(perk.currentLevel)) << ")!" << Color::RESET;
                            usleep(800000);
                        } else {
                            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough stars! Need " 
                                 << cost << " ★" << Color::RESET;
                            usleep(800000);
                        }
                    } else {
                        cout << "\n  " << Color::YELLOW << "Already at max level!" << Color::RESET;
                        usleep(800000);
                    }
                }
            } else if (choice == 'q' || choice == 'Q' || choice == 'a' || choice == 'A') {
                inShop = false;
            }
        }
    }
    
    /*
     * displaySpecializationMenu() - Choose a specialization build (Phase C)
     */
    void displaySpecializationMenu() {
        clearScreen();
        printLine('=', 50, Color::CYAN);
        cout << Color::BOLD << Color::CYAN;
        printCentered("SPECIALIZATION", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::CYAN);
        
        cout << "\n  " << Color::WHITE << "Choose your casino specialty for bonus payouts!" << Color::RESET << "\n";
        cout << "  " << Color::DIM << "You can change this anytime, but bonuses only apply to your specialty." << Color::RESET << "\n\n";
        
        const char* specNames[] = {"None", "Wheel Tycoon", "Slot Mogul", "Card Shark", "Roulette Baron"};
        const char* specDesc[] = {
            "No specialization bonus",
            "+50% payout on Wheel spins",
            "+50% payout on Slots",
            "+50% payout on Blackjack",
            "+50% payout on Roulette"
        };
        const char* specIcons[] = {"", "🎡", "🎰", "🃏", "🎲"};
        
        for (int i = 0; i <= 4; i++) {
            if (i == specializationId) {
                cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD << " ✓ ";
            } else {
                cout << "    ";
            }
            
            cout << Color::CYAN << "[" << i << "]" << Color::RESET << " ";
            if (i > 0) cout << specIcons[i] << " ";
            cout << Color::WHITE << specNames[i] << Color::RESET;
            
            if (i == specializationId) {
                cout << Color::BG_GREEN << " " << Color::RESET;
            }
            cout << "\n";
            
            cout << "      " << Color::DIM << specDesc[i] << Color::RESET << "\n\n";
        }
        
        cout << "  " << Color::CYAN << "[Q]" << Color::RESET << " Back to Main Menu\n";
        cout << "\n  Choice: ";
        
        char choice = getch();
        
        if (choice >= '0' && choice <= '4') {
            int newSpec = choice - '0';
            if (newSpec != specializationId) {
                specializationId = newSpec;
                cout << "\n  " << Color::GREEN << UI::CHECK << " Specialization changed to: " 
                     << specNames[newSpec] << Color::RESET;
                usleep(1000000);
            }
        }
    }
    
    /*
     * calculatePrestigeStars() - Calculate stars earned from prestige (Phase B)
     * 
     * Formula: base 5 stars + 1 per prestige level + bonus from lifetime earnings
     */
    int calculatePrestigeStars() const {
        return economy::prestigeStars(prestigeLevel, totalLifetimeEarnings);
    }
    
    /*
     * getStartingMoneyFromPerk() - Calculate starting money based on perk level (Phase B)
     */
    double getStartingMoneyFromPerk() const {
        return economy::startingMoneyFromPerk(perkStartingMoney);
    }
    
    /*
     * performPrestige() - Execute the prestige reset
     */
    void performPrestige() {
        // Confirm prestige
        cout << "\n\n  " << Color::YELLOW << "Are you sure? This will reset your progress!" 
             << Color::RESET << "\n";
        cout << "  " << Color::CYAN << "[Y]" << Color::RESET << " Yes  "
             << Color::CYAN << "[N]" << Color::RESET << " No\n";
        cout << "  Choice: ";
        
        char confirm = getch();
        if (confirm != 'y' && confirm != 'Y') {
            return;
        }
        
        // Calculate prestige stars earned (Phase B)
        int starsEarned = calculatePrestigeStars();
        prestigeStars += starsEarned;
        
        // Perform prestige
        prestigeLevel++;
        prestigeMultiplier += 0.1;
        totalLifetimeEarnings += lifetimeEarnings;
        
        // Reset progress (apply starting money perk)
        money = getStartingMoneyFromPerk();
        betAmount = 10.0;
        round = 1;
        totalWins = 0;
        totalLosses = 0;
        peakMoney = money;
        lifetimeEarnings = 0.0;
        currentWinStreak = 0;
        
        // Reset generators
        for (auto& gen : generators) {
            gen.setOwned(0);
        }
        
        // Reset upgrades
        for (auto& upg : upgrades) {
            upg.setLevel(0);
        }
        
        // Check prestige achievements (Phase B)
        checkPrestigeAchievements();
        
        // Show success message
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('*', 50, Color::YELLOW);
        printCentered("PRESTIGE COMPLETE!", 50, Color::YELLOW + Color::BOLD);
        printLine('*', 50, Color::YELLOW);
        
        cout << "\n  " << Color::WHITE << "You are now Prestige Level " << Color::YELLOW 
             << prestigeLevel << Color::RESET << "!\n";
        cout << "  " << Color::WHITE << "New income multiplier: " << Color::GREEN 
             << fixed << setprecision(1) << prestigeMultiplier << "x" << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Prestige stars earned: " << Color::YELLOW 
             << "+" << starsEarned << " ★" << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Total stars: " << Color::YELLOW 
             << prestigeStars << " ★" << Color::RESET << "\n";
        
        cout << "\n  Press any key to continue...";
        getch();
        
        saveGame();
    }
    
    // ========================================================================
    // ACHIEVEMENT SYSTEM (Sprint 4 - Polish)
    // ========================================================================
    
    /*
     * showAchievement() - Display achievement unlock notification
     */
    void showAchievement(const string& name, const string& desc) {
        cout << "\n";
        cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD 
             << " ★ ACHIEVEMENT UNLOCKED! ★ " << Color::RESET << "\n";
        cout << "  " << Color::YELLOW << name << Color::RESET << "\n";
        cout << "  " << Color::DIM << desc << Color::RESET << "\n";
        usleep(1500000);
    }
    
    /*
     * updateAchievementMultiplier() - Recalculate achievement bonus (Phase B)
     * 
     * Each achievement gives +1% to all passive income.
     */
    void updateAchievementMultiplier() {
        achievementMultiplier = economy::achievementMultiplier(getAchievementCount());
    }
    
    /*
     * checkAchievements() - Check and unlock any earned achievements
     * 
     * Called after wins and purchases to check for new achievements.
     */
    void checkAchievements() {
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
    void checkHighRollerAchievement(double winAmount) {
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
    void checkPrestigeAchievements() {
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
     * displayAchievements() - Show all achievements and their status
     */
    void displayAchievements() {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('=', 50, Color::YELLOW);
        printCentered("ACHIEVEMENTS", 50, Color::YELLOW + Color::BOLD);
        printLine('=', 50, Color::YELLOW);
        
        int unlocked = 0;
        int total = 8;
        
        auto showAch = [&](bool earned, const string& name, const string& desc) {
            if (earned) {
                unlocked++;
                cout << "  " << Color::GREEN << "★ " << Color::RESET;
            } else {
                cout << "  " << Color::DIM << "☆ " << Color::RESET;
            }
            cout << (earned ? Color::WHITE : Color::DIM) << name << Color::RESET;
            cout << "\n    " << Color::DIM << desc << Color::RESET << "\n\n";
        };
        
        cout << "\n";
        showAch(achFirstWin, "First Win", "Win your first game");
        showAch(achHighRoller, "High Roller", "Win $1,000 in a single bet");
        showAch(achMillionaire, "Millionaire", "Reach $1,000,000");
        showAch(achLuckyStreak, "Lucky Streak", "Win 10 games in a row");
        showAch(achGenerator10, "Generator Tycoon", "Own 10 of any generator");
        showAch(achAllUpgrades, "Fully Upgraded", "Max out all upgrades");
        showAch(achPrestige1, "First Prestige", "Prestige for the first time");
        showAch(achPrestige5, "Prestige Master", "Reach prestige level 5");
        
        printLine('-', 50, Color::DIM);
        cout << "\n  " << Color::WHITE << "Progress: " << Color::YELLOW 
             << unlocked << "/" << total << Color::RESET << " achievements unlocked\n";
        
        cout << "\n  Press any key to continue...";
        getch();
    }
    
    /*
     * recordWin() - Record a win and update streak
     */
    void recordWin(double winAmount) {
        totalWins++;
        currentWinStreak++;
        updatePeakMoney();
        checkHighRollerAchievement(winAmount);
        checkAchievements();
    }
    
    /*
     * recordLoss() - Record a loss and reset streak
     */
    void recordLoss() {
        totalLosses++;
        currentWinStreak = 0;
    }
    
    // ========================================================================
    // GENERATOR SHOP (Sprint 1 - Core Incremental Feature)
    // ========================================================================
    
    /*
     * getGeneratorCostMultiplier() - Get cost reduction from prestige perks (Phase B)
     */
    double getGeneratorCostMultiplier() const {
        return economy::generatorCostMultiplier(perkGeneratorDiscount);
    }
    
    double getGeneratorBulkCost(const Generator& gen, int amount) const {
        return economy::bulkCost(gen, amount, getGeneratorCostMultiplier());
    }

    int getGeneratorMaxAffordable(const Generator& gen, double availableMoney) const {
        return economy::maxAffordable(gen, availableMoney, getGeneratorCostMultiplier());
    }
    
    /*
     * displayGeneratorShop() - Show the generator purchase menu
     * 
     * This is where players spend money to buy passive income generators.
     * Each generator produces money automatically over time.
     * 
     * The shop displays:
     * - Generator name and how many owned
     * - Cost to buy the next one
     * - Income per second each one provides
     */
    void displayGeneratorShop() {
        bool inShop = true;
        int shopSelection = 0;  // 0 = back, 1-8 = generators
        int maxSelection = generators.size();  // 0 to maxSelection

        int buyMode = 0;
        static const int BUY_MODE_COUNT = 5;
        const int buyAmounts[BUY_MODE_COUNT] = {1, 10, 25, 100, 0};
        const string buyModeNames[BUY_MODE_COUNT] = {"x1", "x10", "x25", "x100", "MAX"};
         
        while (inShop) {
            // Update passive income while in shop
            updatePassiveIncome();
            
            clearScreen();
            printLine('=', 50, Color::MAGENTA);
            cout << Color::BOLD << Color::MAGENTA;
            printCentered("GENERATOR SHOP", 50);
            cout << Color::RESET;
            printLine('=', 50, Color::MAGENTA);
            
            // Show current balance and income
            cout << "\n  " << Color::WHITE << "Your Balance: " << Color::GREEN 
                 << formatMoney(money) << Color::RESET << "\n";
            cout << "  " << Color::WHITE << "Current Income: " << Color::GREEN 
                 << "+$" << fixed << setprecision(2) << getTotalPassiveIncome() 
                 << "/sec" << Color::RESET << "\n";
            cout << "  " << Color::WHITE << "Buy Mode: " << Color::CYAN 
                 << buyModeNames[buyMode] << Color::RESET << "  " << Color::DIM
                 << "(press E to change)" << Color::RESET << "\n";
            cout << "  " << Color::DIM << "Use W/S to navigate, Enter to buy, E to change quantity, Q to exit" 
                 << Color::RESET << "\n\n";
             
            printLine('-', 50, Color::DIM);
             
            // List all generators with cursor
            for (size_t i = 0; i < generators.size(); i++) {
                Generator& gen = generators[i];

                int buyAmount = buyAmounts[buyMode];
                int amountToBuy = buyAmount;
                double cost = 0.0;

                if (buyAmount == 0) {
                    amountToBuy = getGeneratorMaxAffordable(gen, money);
                    if (amountToBuy > 0) {
                        cost = getGeneratorBulkCost(gen, amountToBuy);
                    } else {
                        cost = gen.getCurrentCost();
                    }
                } else {
                    cost = getGeneratorBulkCost(gen, buyAmount);
                }

                bool canAfford = (buyAmount == 0) ? (amountToBuy > 0 && money >= cost) : (money >= cost);
                string costColor = canAfford ? Color::GREEN : Color::RED;

                double perUnitIncome = gen.getBaseIncome() * getIncomeMultiplier() * prestigeMultiplier;
                double gainPerSec = perUnitIncome * ((buyAmount == 0) ? amountToBuy : buyAmount);

                string buyLabel;
                if (buyAmount == 0) {
                    buyLabel = "MAX x" + to_string(amountToBuy);
                } else {
                    buyLabel = "x" + to_string(buyAmount);
                }
                 
                // Highlight selected item
                if ((int)i + 1 == shopSelection) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                    cout << gen.getIcon() << " " << gen.getName() << Color::RESET;
                } else {
                    cout << "    " << gen.getIcon() << " " << Color::WHITE << gen.getName() << Color::RESET;
                }
                
                cout << " x" << gen.getOwned();
                cout << " " << Color::GREEN << "(+$" << fixed << setprecision(1) 
                     << gen.getBaseIncome() << "/s)" << Color::RESET << "\n";
                 
                cout << "      " << Color::DIM << gen.getDescription() << Color::RESET << "\n";
                cout << "      Buy: " << Color::CYAN << buyLabel << Color::RESET;
                cout << "  Cost: " << costColor << formatMoney(cost) << Color::RESET;
                cout << "  Gain: " << Color::GREEN << "+" << formatMoney(gainPerSec) << "/sec" << Color::RESET << "\n";
            }
            
            printLine('-', 50, Color::DIM);
            
            // Back option
            if (shopSelection == 0) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << "Back to Main Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::WHITE << "Back to Main Menu" << Color::RESET << "\n";
            }
            
            // Get user input with timeout for real-time updates
            char choice = getchWithTimeout(1000);
            
            if (choice == 0) continue;  // Timeout, refresh
            
            // Handle arrow keys
            if (choice == 27) {
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';
                    else if (seq2 == 'B') choice = 's';
                }
                if (choice == 27) continue;
            }
            
            // Navigation
            if (choice == 'w' || choice == 'W') {
                shopSelection = (shopSelection - 1 + maxSelection + 1) % (maxSelection + 1);
            } else if (choice == 's' || choice == 'S') {
                shopSelection = (shopSelection + 1) % (maxSelection + 1);
            } else if (choice == 'e' || choice == 'E') {
                buyMode = (buyMode + 1) % BUY_MODE_COUNT;
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (shopSelection == 0) {
                    inShop = false;
                } else {
                    purchaseGenerator(shopSelection - 1, buyAmounts[buyMode]);
                }
            } else if (choice == 'q' || choice == 'Q' || choice == 'a' || choice == 'A') {
                inShop = false;
            } else if (choice >= '1' && choice <= '0' + (int)generators.size()) {
                purchaseGenerator(choice - '1', buyAmounts[buyMode]);
            } else if (choice == '0') {
                inShop = false;
            }
        }
    }
    
    /*
     * purchaseGenerator() - Attempt to buy a generator
     * 
     * Parameters:
     *   - index: Index of the generator in the generators vector
     * 
     * This method:
     * 1. Checks if the player can afford the generator
     * 2. Deducts the cost from player's money
     * 3. Adds one to the generator's owned count
     */
    void purchaseGenerator(int index, int amount = 1) {
        // Bounds checking - make sure index is valid
        if (index < 0 || index >= (int)generators.size()) {
            return;
        }
        
        Generator& gen = generators[index];

        int purchaseAmount = amount;
        if (purchaseAmount < 0) purchaseAmount = 1;

        if (purchaseAmount == 0) {
            purchaseAmount = getGeneratorMaxAffordable(gen, money);
            if (purchaseAmount <= 0) {
                double singleCost = gen.getCurrentCost();
                cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need " 
                     << formatMoney(singleCost) << Color::RESET;
                usleep(800000);
                return;
            }
        }

        if (purchaseAmount > INT_MAX - gen.getOwned()) {
            purchaseAmount = INT_MAX - gen.getOwned();
        }
        if (purchaseAmount <= 0) {
            return;
        }

        double cost = getGeneratorBulkCost(gen, purchaseAmount);
         
        if (money >= cost) {
            money -= cost;
            gen.setOwned(gen.getOwned() + purchaseAmount);
             
            cout << "\n  " << Color::GREEN << UI::CHECK << " Purchased ";
            if (purchaseAmount == 1) {
                cout << gen.getName();
            } else {
                cout << purchaseAmount << "x " << gen.getName();
            }
            cout << "!" << Color::RESET;
            cout << "\n  " << Color::DIM << "You now own " << gen.getOwned() 
                 << " and earn +$" << fixed << setprecision(2) 
                 << gen.getIncomePerSecond() << "/sec from them." << Color::RESET;
             
            usleep(800000);
        } else {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need " 
                 << formatMoney(cost) << Color::RESET;
            usleep(800000);
        }
    }
    
    /*
     * confirmRestart() - Ask for confirmation before resetting all progress
     */
    void confirmRestart() {
        clearScreen();
        printLine('=', 50, Color::YELLOW);
        cout << Color::BOLD << Color::YELLOW;
        printCentered("RESTART GAME", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::YELLOW);
        
        cout << "\n";
        printCentered("Are you sure you want to restart?", 50, Color::WHITE);
        cout << "\n";
        cout << "  " << Color::RED << "WARNING: " << Color::RESET 
             << "This will delete ALL progress including:\n";
        cout << "    - Money and bet amount\n";
        cout << "    - All generators and upgrades\n";
        cout << "    - Prestige level and multiplier\n";
        cout << "    - Statistics and achievements\n";
        cout << "    - Save file\n";
        cout << "\n";
        printCentered("This action CANNOT be undone!", 50, Color::RED + Color::BOLD);
        cout << "\n\n";
        cout << "  " << Color::GREEN << "[Y]" << Color::RESET << " Yes, restart from scratch\n";
        cout << "  " << Color::RED << "[N]" << Color::RESET << " No, keep playing\n";
        cout << "\n  Your choice: ";
        
        char choice = getch();
        
        if (choice == 'y' || choice == 'Y') {
            resetGame();
            cout << "\n\n  " << Color::GREEN << UI::CHECK << " Game has been reset!" << Color::RESET;
            cout << "\n  " << Color::DIM << "Starting fresh with $100..." << Color::RESET;
            usleep(2000000);
        } else {
            cout << "\n\n  " << Color::CYAN << "Restart cancelled." << Color::RESET;
            usleep(1000000);
        }
    }
    
    /*
     * resetGame() - Reset all game state to initial values
     */
    void resetGame() {
        // Reset money and betting
        money = 100.0;
        betAmount = 10.0;
        peakMoney = 100.0;
        
        // Reset statistics
        round = 0;
        totalWins = 0;
        totalLosses = 0;
        currentWinStreak = 0;
        
        // Reset prestige
        prestigeLevel = 0;
        prestigeMultiplier = 1.0;
        lifetimeEarnings = 0.0;
        totalLifetimeEarnings = 0.0;
        
        // Reset generators
        for (auto& gen : generators) {
            gen.reset();
        }
        
        // Reset upgrades
        for (auto& upg : upgrades) {
            upg.reset();
        }
        
        // Reset achievements
        achFirstWin = false;
        achHighRoller = false;
        achLuckyStreak = false;
        achMillionaire = false;
        achPrestige1 = false;
        achPrestige5 = false;
        achGenerator10 = false;
        achAllUpgrades = false;
        
        // Reset menu and animation state
        menuSelection = 0;
        animationFrame = 0;
        
        // Reset time tracking
        lastUpdateTime = time(0);
        lastSaveTime = time(0);
        
        // Delete save file (and its backup, so a restart really is a restart)
        deleteSaveFile();
    }
    
    /*
     * displayGameOver() - Show game over screen when player goes bankrupt
     */
    void displayGameOver() {
        clearScreen();
        printLine('=', 50, Color::RED);
        cout << Color::BOLD << Color::RED;
        printCentered("GAME OVER", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::RED);
        
        cout << "\n";
        printCentered("You went bankrupt!", 50, Color::RED);
        cout << "\n";
        cout << "  " << Color::WHITE << "Final Statistics:" << Color::RESET << "\n";
        cout << "    Rounds played: " << Color::CYAN << round << Color::RESET << "\n";
        cout << "    Total wins: " << Color::GREEN << totalWins << Color::RESET << "\n";
        cout << "    Total losses: " << Color::RED << totalLosses << Color::RESET << "\n";
        cout << "    Peak balance: " << Color::YELLOW << "$" << fixed 
             << setprecision(2) << peakMoney << Color::RESET << "\n";
        cout << "\n";
        printCentered("Better luck next time!", 50, Color::DIM);
        cout << "\n";
    }
    
    /*
     * displayExitScreen() - Show exit screen when player quits
     */
    void displayExitScreen() {
        clearScreen();
        printLine('=', 50, Color::GREEN);
        cout << Color::BOLD << Color::GREEN;
        printCentered("THANKS FOR PLAYING!", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::GREEN);
        
        cout << "\n";
        cout << "  " << Color::WHITE << "Final Statistics:" << Color::RESET << "\n";
        cout << "    Final balance: " << Color::GREEN << "$" << fixed 
             << setprecision(2) << money << Color::RESET << "\n";
        cout << "    Rounds played: " << Color::CYAN << round << Color::RESET << "\n";
        cout << "    Total wins: " << Color::GREEN << totalWins << Color::RESET << "\n";
        cout << "    Total losses: " << Color::RED << totalLosses << Color::RESET << "\n";
        cout << "    Peak balance: " << Color::YELLOW << "$" << fixed 
             << setprecision(2) << peakMoney << Color::RESET << "\n";
        
        // Calculate and display profit/loss
        double profit = money - 100.0;  // Started with $100
        cout << "    Net profit: ";
        if (profit >= 0) {
            cout << Color::GREEN << "+$" << fixed << setprecision(2) << profit;
        } else {
            cout << Color::RED << "-$" << fixed << setprecision(2) << (-profit);
        }
        cout << Color::RESET << "\n\n";
    }
    
    // ========================================================================
    // MAIN GAME LOOP
    // ========================================================================
    
    /*
     * play() - Main game loop
     * 
     * This is the entry point for the game. It:
     * 1. Updates passive income based on elapsed time
     * 2. Displays the current game state
     * 3. Waits for player input (with timeout for continuous updates)
     * 4. Processes the input (play game, buy generators, change bet, quit)
     * 5. Checks for game over condition
     * 6. Repeats until player quits or goes bankrupt
     * 
     * The while loop continues as long as:
     * - quit is false (player hasn't chosen to quit)
     * - money > 0 OR passive income > 0 (can recover with generators)
     */
    /*
     * dispatchMenuAction() - Run the action for a menu entry
     *
     * The switch covers every MenuAction value, so -Wswitch flags any entry
     * added to MENU_TABLE without a handler here.
     *
     * Returns: true when the player chose to quit
     */
    bool dispatchMenuAction(MenuAction action) {
        switch (action) {
            case MenuAction::SpinWheel:      spinWheel(); break;
            case MenuAction::PlaySlots:      playSlots(); break;
            case MenuAction::Blackjack:      playBlackjack(); break;
            case MenuAction::Roulette:       playRoulette(); break;
            case MenuAction::GeneratorShop:  displayGeneratorShop(); break;
            case MenuAction::UpgradeShop:    displayUpgradeShop(); break;
            case MenuAction::ChangeBet:      changeBet(); break;
            case MenuAction::Prestige:       displayPrestigeMenu(); break;
            case MenuAction::Specialization: displaySpecializationMenu(); break;
            case MenuAction::Achievements:   displayAchievements(); break;
            case MenuAction::Save:
                // Manual save (Sprint 3)
                if (saveGame()) {
                    cout << "\n  " << Color::GREEN << UI::CHECK << " Game saved!" << Color::RESET;
                    usleep(1000000);
                }
                break;
            case MenuAction::Restart:
                // Restart game from scratch
                confirmRestart();
                break;
            case MenuAction::Quit:
                return true;
        }
        return false;
    }

    void play() {
        char choice;      // Store the player's menu selection
        bool quit = false; // Flag to track if player wants to quit
        
        // Main game loop - continues while not quitting
        // With passive income, player can recover even from $0
        while (!quit) {
            // Update passive income before displaying
            // This is crucial for the incremental game feel
            updatePassiveIncome();
            
            displayStatus();
            
            // Use non-blocking input with timeout for real-time updates (Sprint 6)
            // This allows the balance to update visually every second
            choice = getchWithTimeout(1000);  // 1 second timeout
            
            // If timeout (no input), just refresh the display
            if (choice == 0) {
                continue;
            }
            
            // Handle arrow keys (escape sequences) and WASD navigation (Sprint 7)
            // Arrow keys send: ESC [ A (up), ESC [ B (down)
            if (choice == 27) {  // ESC character
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';  // Up arrow -> w
                    else if (seq2 == 'B') choice = 's';  // Down arrow -> s
                }
                if (choice == 27) continue;  // Just ESC pressed, ignore
            }
            
            // Navigation controls (Sprint 7)
            if (choice == 'w' || choice == 'W') {
                menuSelection = (menuSelection - 1 + MENU_ITEM_COUNT) % MENU_ITEM_COUNT;
                continue;
            }
            if (choice == 's' || choice == 'S') {
                // S always navigates down (use Enter to select Save)
                menuSelection = (menuSelection + 1) % MENU_ITEM_COUNT;
                continue;
            }
            if (choice == 'a' || choice == 'A') {
                // A can be used to go back/cancel in future
                continue;
            }
            // D, Enter, or Space selects the highlighted menu item
            if (choice == 'd' || choice == 'D' || choice == '\n' || choice == '\r' || choice == ' ') {
                quit = dispatchMenuAction(MENU_TABLE[menuSelection].action);
            }
            // Digits 0-9 jump straight to the first ten menu entries
            else if (choice >= '0' && choice <= '9') {
                int index = choice - '0';
                if (index < MENU_ITEM_COUNT) {
                    quit = dispatchMenuAction(MENU_TABLE[index].action);
                }
            }
            // Quick-access keys for the entries past the digit range. 's' is
            // deliberately not one: the navigation branch above claims it for
            // "down", so Save Game is selected with Enter on its row.
            else if (choice == 'r' || choice == 'R') {
                confirmRestart();
            }
            else if (choice == 'q' || choice == 'Q') {
                quit = true;
            }
            // Any other input is ignored - just refresh the display
            
            // Auto-save periodically (Sprint 3)
            autoSave();
            
            // Check for bankruptcy (only if no passive income to save them)
            if (money <= 0 && getTotalPassiveIncome() <= 0) {
                displayGameOver();
                cout << "\n  " << Color::YELLOW << "Press any key to start a new game..." << Color::RESET;
                getch();
                resetGame();  // Auto-reset so player can start fresh
                // Don't quit - continue playing with fresh start
            }
        }
        
        // Save game before exiting (Sprint 3)
        saveGame();
        
        // Show exit screen if player quit with money remaining
        if (money > 0) {
            displayExitScreen();
        }
    }
};

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================
/*
 * Static member variables must be defined outside the class.
 * This is because static members are shared across all instances and need
 * a single definition in memory.
 */
const string GamblingGame::SAVE_FILENAME = "lucky_spin_save.txt";

// ============================================================================
// MAIN FUNCTION
// ============================================================================
/*
 * main() - Program entry point
 * 
 * Every C++ program must have exactly one main() function.
 * This is where execution begins.
 * 
 * Returns: 0 indicates successful execution
 *          Non-zero values indicate errors
 */
int main() {
    /*
     * Snapshot the terminal settings BEFORE anything switches it into raw mode,
     * and register handlers to put them back. Without this, quitting with
     * Ctrl+C leaves the user's shell with no echo and no line editing.
     */
    terminal::init();
    
    try {
        // Create an instance (object) of the GamblingGame class
        // This calls the constructor, initializing all member variables
        GamblingGame game;
        
        // Try to load existing save file (Sprint 3)
        // If successful, offline earnings will be calculated and shown
        game.loadGame();
        
        // Start the game by calling the play() method
        game.play();
    }
    catch (const std::exception& e) {
        /*
         * Restore the terminal before printing, otherwise the message renders
         * into a raw-mode terminal and the shell is left unusable.
         */
        terminal::restore();
        std::cerr << "\nLucky Spin hit an unexpected error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        terminal::restore();
        std::cerr << "\nLucky Spin hit an unexpected error.\n";
        return 1;
    }
    
    // Return 0 to indicate successful program completion
    return 0;
}
