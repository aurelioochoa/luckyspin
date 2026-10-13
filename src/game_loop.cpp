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
// MAIN LOOP + MENU DISPATCH
// ==========================================================================

/*
 * confirmRestart() - Ask for confirmation before resetting all progress
 */
void GamblingGame::confirmRestart() {
    clearScreen(); displayHeader();
    terminal::line(Color::RED + Color::BOLD + "RESET ALL PROGRESS?");
    terminal::rule();
    terminal::line("This clears your entire casino:");
    terminal::line("Money, generators, and upgrades");
    terminal::line("Prestige, stars, and permanent perks");
    terminal::line("Achievements and your active hand");
    terminal::line(Color::DIM + "You start over with $100. This cannot be undone.");
    terminal::rule();
    terminal::line(Color::RED + "Y  Reset everything");
    terminal::line(Color::GREEN + "Any other key  Keep playing");
    terminal::finishScreen();
    const char key = getch();
    if (key == 'y' || key == 'Y') { resetGame(); saveGame(); }
}

/*
 * resetGame() - Reset all game state to initial values
 */
void GamblingGame::resetGame() {
    // Reset money and betting
    money = 100.0;
    betAmount = 10.0;
    peakMoney = 100.0;
    
    // Reset statistics
    round = 1;
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
    achPlinkoJackpot = false;
    achievementMultiplier = 1.0;
    prestigeStars = 0;
    perkStartingMoney = perkGeneratorDiscount = perkOfflineCap = perkPassiveBoost = 0;
    specializationId = 0;
    interestUnlocked = false;
    interestRate = 0.001;
    for (int i = 0; i < 10; ++i) milestoneUnlocked[i] = false;
    activeEventId = economy::EVENT_NONE;
    eventEndTime = 0;
    nextEventTime = time(0) + economy::EVENT_MIN_GAP_SECONDS
                  + Rng::global().range(0, economy::EVENT_GAP_SPREAD_SECONDS);
    bjPhase = BlackjackPhase::Idle;
    bjPlayer.clear();
    bjDealer.clear();
    bjBet = 0.0;
    bjSettled = BlackjackSettlement{};
    shoe.shuffle(Rng::global());
    notificationQueue.clear();
    
    // Reset menu and animation state
    menuSelection = 0;
    animationFrame = 0;
    
    // Reset time tracking
    lastUpdateTime = time(0);
    lastSaveTime = time(0);
    gameStartTime = time(0);
    
    // Delete save file (and its backup, so a restart really is a restart)
    deleteSaveFile();
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
bool GamblingGame::dispatchMenuAction(MenuAction action) {
    switch (action) {
        case MenuAction::SpinWheel:      spinWheel(); break;
        case MenuAction::PlaySlots:      playSlots(); break;
        case MenuAction::Blackjack:      playBlackjack(); break;
        case MenuAction::Roulette:       playRoulette(); break;
        case MenuAction::Plinko:         playPlinko(); break;
        case MenuAction::GeneratorShop:  displayGeneratorShop(); break;
        case MenuAction::UpgradeShop:    displayUpgradeShop(); break;
        case MenuAction::ChangeBet:      changeBet(); break;
        case MenuAction::Prestige:       displayPrestigeMenu(); break;
        case MenuAction::Specialization: displaySpecializationMenu(); break;
        case MenuAction::Achievements:   displayAchievements(); break;
        case MenuAction::About:          displayAbout(); break;
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

void GamblingGame::play() {
    char choice;      // Store the player's menu selection
    bool quit = false; // Flag to track if player wants to quit
    
    // Main game loop - continues while not quitting
    // With passive income, player can recover even from $0
    while (!quit) {
        // Update passive income before displaying
        // This is crucial for the incremental game feel
        updatePassiveIncome();
        autoSave();
        
        displayStatus();
        
        // Use non-blocking input with timeout for real-time updates (Sprint 6)
        // This allows the balance to update visually every second
        choice = getMenuInput(1000);
        
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
        else if (choice == '?') {
            displayAbout();
        }
        else if (choice == 'v' || choice == 'V') {
            dispatchMenuAction(MenuAction::Save);
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
