/*
 * gambling_game.h - Declaration of the GamblingGame class.
 *
 * Member variables and small inline helpers live here; the method
 * bodies are split across src/game_*.cpp by topic:
 *   game_init.cpp    construction + generator/upgrade definitions
 *   game_economy.cpp economy glue (delegates to core/economy.h)
 *   game_display.cpp terminal rendering
 *   game_games.cpp   playable games
 *   game_shops.cpp   shops, prestige, specialization
 *   game_save.cpp    persistence (save/load/parsing)
 *   game_loop.cpp    main loop + menu dispatch
 */

#ifndef GAMBLING_GAME_H
#define GAMBLING_GAME_H

#include <string>
#include <vector>
#include <ctime>

#include "menu.h"           // MenuAction, MenuEntry, MENU_TABLE
#include "core/generator.h" // Generator member vector
#include "core/upgrade.h"   // Upgrade member vector
#include "core/deck.h"      // cards::Shoe member
#include "core/save_path.h" // savepath::resolve() used by savePath()

// The class below was written against unqualified std names; keeping
// this here avoids touching every declaration.
using namespace std;

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

    // ---- game_init.cpp ----
    GamblingGame();
    void initializeGenerators();
    void initializeUpgrades();

    // ---- game_economy.cpp ----
    double getUpgradeValue(UpgradeType type) const;
    double getIncomeMultiplier() const;
    int getAchievementCount() const;
    double getPassiveBoostFromPerks() const;
    int getTotalGeneratorsOwned() const;
    double getGeneratorSynergyMultiplier(int genIndex) const;
    double getSpecializationMultiplier() const;
    double getTotalPassiveIncome() const;
    void checkMilestones();
    void updatePassiveIncome();
    void updatePeakMoney();
    bool canAffordBet();
    double applyLuckyBonus(double amount);
    void applyOfflineEarnings(time_t savedTimestamp);
    double getPrestigeRequirement() const;
    bool canPrestige() const;
    int calculatePrestigeStars() const;
    double getStartingMoneyFromPerk() const;
    void updateAchievementMultiplier();
    void checkAchievements();
    void checkHighRollerAchievement(double winAmount);
    void checkPrestigeAchievements();
    void recordWin(double winAmount);
    void recordLoss();
    double getGeneratorCostMultiplier() const;
    double getGeneratorBulkCost(const Generator& gen, int amount) const;
    int getGeneratorMaxAffordable(const Generator& gen, double availableMoney) const;

    // ---- game_display.cpp ----
    void displayHeader();
    void showMilestone(const string& name, const string& desc);
    void displayStats();
    string menuDesc(MenuAction action) const;
    string specializationName() const;
    void displayMenu();
    void displayStatus();
    void showAchievement(const string& name, const string& desc);
    void displayAchievements();
    void displayGameOver();
    void displayExitScreen();

    // ---- game_games.cpp ----
    void changeBet();
    void spinWheel();
    string getColoredSymbol(int index);
    void playSlots();
    void playBlackjack();
    void playRoulette();

    // ---- game_shops.cpp ----
    void displayUpgradeShop();
    void purchaseUpgrade(int index);
    void displayPrestigeMenu();
    void displayPrestigeShop();
    void displaySpecializationMenu();
    void performPrestige();
    void displayGeneratorShop();
    void purchaseGenerator(int index, int amount = 1);

    // ---- game_save.cpp ----
    bool saveGame();
    static bool parseDouble(const string& text, double& out);
    static bool parseInt(const string& text, int& out);
    static bool parseLong(const string& text, long& out);
    static bool parseBool(const string& text, bool& out);
    static bool parseMoney(const string& text, double& out);
    static bool parseCount(const string& text, int& out, int maxValue);
    bool loadGame();
    bool loadFrom(const string& path, time_t& savedTimestamp);
    void autoSave();
    void deleteSaveFile();

    // ---- game_loop.cpp ----
    void confirmRestart();
    void resetGame();
    bool dispatchMenuAction(MenuAction action);
    void play();
};

#endif // GAMBLING_GAME_H
