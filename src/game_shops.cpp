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
// SHOPS, PRESTIGE AND SPECIALIZATION
// ==========================================================================

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
void GamblingGame::displayUpgradeShop() {
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
void GamblingGame::purchaseUpgrade(int index) {
    double cost = 0.0;
    if (buyUpgradeLevel(index, cost)) {
        const Upgrade& upg = upgrades[index];
        cout << "\n  " << Color::GREEN << UI::CHECK << " Upgraded " << upg.getName()
             << " to Lv." << upg.getCurrentLevel() << "!" << Color::RESET;
        cout << "\n  " << Color::DIM << "New value: " << fixed << setprecision(1)
             << upg.getCurrentValue() << Color::RESET;
        usleep(800000);
    } else if (index >= 0 && index < (int)upgrades.size()) {
        const Upgrade& upg = upgrades[index];
        if (upg.isMaxed()) {
            cout << "\n  " << Color::YELLOW << "Already at max level!" << Color::RESET;
            usleep(800000);
        } else {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need "
                 << formatMoney(cost) << Color::RESET;
            usleep(800000);
        }
    }
}

/*
 * displayPrestigeMenu() - Show prestige information and options
 */
void GamblingGame::displayPrestigeMenu() {
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
void GamblingGame::displayPrestigeShop() {
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
                // Purchase through the shared headless action, then report.
                PerkInfo& perk = perks[shopSelection];
                int cost = 0;
                if (buyPerkLevel(shopSelection, cost)) {
                    cout << "\n  " << Color::GREEN << UI::CHECK << " Purchased "
                         << perk.name << " (Level " << (*(perk.currentLevel)) << ")!" << Color::RESET;
                    usleep(800000);
                } else if (*(perk.currentLevel) >= perk.maxLevel) {
                    cout << "\n  " << Color::YELLOW << "Already at max level!" << Color::RESET;
                    usleep(800000);
                } else {
                    cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough stars! Need "
                         << cost << "\xE2\x98\x85" << Color::RESET;
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
void GamblingGame::displaySpecializationMenu() {
    clearScreen();
    printLine('=', 50, Color::CYAN);
    cout << Color::BOLD << Color::CYAN;
    printCentered("SPECIALIZATION", 50);
    cout << Color::RESET;
    printLine('=', 50, Color::CYAN);
    
    cout << "\n  " << Color::WHITE << "Choose your casino specialty for bonus payouts!" << Color::RESET << "\n";
    cout << "  " << Color::DIM << "You can change this anytime, but bonuses only apply to your specialty." << Color::RESET << "\n\n";
    
    const char* specNames[] = {"None", "Wheel Tycoon", "Slot Mogul", "Card Shark", "Roulette Baron", "Plinko Pro"};
    const char* specDesc[] = {
        "No specialization bonus",
        "+50% payout on Wheel spins",
        "+50% payout on Slots",
        "+50% payout on Blackjack",
        "+50% payout on Roulette",
        "+50% payout on Plinko drops"
    };
    const int SPEC_COUNT = 5;
    const char* specIcons[SPEC_COUNT + 1] = {
        "", "\xF0\x9F\x8E\xA1", "\xF0\x9F\x8E\xB0", "\xF0\x9F\x83\x8F", "\xF0\x9F\x8E\xB2", "\xF0\x9F\x8E\xAF"
    };

    for (int i = 0; i <= SPEC_COUNT; i++) {
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
    
    if (choice >= '0' && choice <= '0' + SPEC_COUNT) {
        int newSpec = choice - '0';
        if (newSpec != specializationId) {
            setSpecializationId(newSpec);
            cout << "\n  " << Color::GREEN << UI::CHECK << " Specialization changed to: "
                 << specNames[newSpec] << Color::RESET;
            usleep(1000000);
        }
    }
}

/*
 * performPrestige() - Confirm, then run the shared prestige core
 */
void GamblingGame::performPrestige() {
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

    const int starsEarned = executePrestige();
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
void GamblingGame::displayGeneratorShop() {
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
void GamblingGame::purchaseGenerator(int index, int amount) {
    double cost = 0.0;
    int bought = 0;

    if (buyGeneratorCount(index, amount, cost, bought)) {
        Generator& gen = generators[index];
        cout << "\n  " << Color::GREEN << UI::CHECK << " Purchased ";
        if (bought == 1) {
            cout << gen.getName();
        } else {
            cout << bought << "x " << gen.getName();
        }
        cout << "!" << Color::RESET;
        cout << "\n  " << Color::DIM << "You now own " << gen.getOwned()
             << " and earn +$" << fixed << setprecision(2)
             << gen.getIncomePerSecond() << "/sec from them." << Color::RESET;
        usleep(800000);
    } else if (index >= 0 && index < (int)generators.size()) {
        // Unaffordable: costOut carries either the bulk price or the single
        // unit price when MAX mode could not afford even one.
        cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need "
             << formatMoney(cost) << Color::RESET;
        usleep(800000);
    }
}
