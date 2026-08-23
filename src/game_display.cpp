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
// TERMINAL RENDERING (header, stats, menu, achievements)
// ==========================================================================

// ========================================================================
// DISPLAY METHODS
// ========================================================================

/*
 * displayHeader() - Show the game title with animated marquee (Sprint 7)
 * 
 * The stars around the title blink like a casino marquee sign.
 * Animation is based on animationFrame counter that increments each refresh.
 */
void GamblingGame::displayHeader() {
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
 * showMilestone() - Display milestone unlock notification (Phase C)
 */
void GamblingGame::showMilestone(const string& name, const string& desc) {
    cout << "\n";
    cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD 
         << " 🏆 MILESTONE UNLOCKED! 🏆 " << Color::RESET << "\n";
    cout << "  " << Color::YELLOW << name << Color::RESET << "\n";
    cout << "  " << Color::DIM << desc << Color::RESET << "\n";
    usleep(1500000);
}

/*
 * displayStats() - Show current game statistics with Unicode decorations
 * 
 * Updated in Sprint 6 to use Unicode icons and show owned generators.
 */
void GamblingGame::displayStats() {
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
string GamblingGame::menuDesc(MenuAction action) const {
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
        case MenuAction::Plinko:          return "(risk levels, 35x edge)";
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
string GamblingGame::specializationName() const {
    switch (specializationId) {
        case 1: return "Wheel";
        case 2: return "Slots";
        case 3: return "Blackjack";
        case 4: return "Roulette";
        case 5: return "Plinko";
        default: return "none";
    }
}

/*
 * displayMenu() - Show the main menu with arrow/WASD navigation (Sprint 7)
 * 
 * Menu items are highlighted based on menuSelection.
 * Use W/S or Up/Down arrows to navigate, Enter/Space to select.
 */
void GamblingGame::displayMenu() {
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
void GamblingGame::displayStatus() {
    clearScreen();
    displayHeader();
    displayStats();
    displayMenu();
}

// ========================================================================
// ACHIEVEMENT SYSTEM (Sprint 4 - Polish)
// ========================================================================

/*
 * showAchievement() - Display achievement unlock notification
 */
void GamblingGame::showAchievement(const string& name, const string& desc) {
    cout << "\n";
    cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD 
         << " ★ ACHIEVEMENT UNLOCKED! ★ " << Color::RESET << "\n";
    cout << "  " << Color::YELLOW << name << Color::RESET << "\n";
    cout << "  " << Color::DIM << desc << Color::RESET << "\n";
    usleep(1500000);
}

/*
 * showEvent() - Display a random-event announcement (Phase D)
 *
 * Mirrors showAchievement(): printed inline at the current cursor position,
 * with a short pause so it is readable before the next redraw.
 */
void GamblingGame::showEvent(const string& name, const string& desc) {
    cout << "\n";
    cout << "  " << Color::BG_MAGENTA << Color::WHITE << Color::BOLD
         << " \xE2\x9A\xA1 CASINO EVENT! \xE2\x9A\xA1 " << Color::RESET << "\n";
    cout << "  " << Color::MAGENTA << name << Color::RESET << "\n";
    cout << "  " << Color::DIM << desc << Color::RESET << "\n";
    usleep(1800000);
}

/*
 * displayAchievements() - Show all achievements and their status
 */
void GamblingGame::displayAchievements() {
    clearScreen();
    displayHeader();
    cout << "\n";
    printLine('=', 50, Color::YELLOW);
    printCentered("ACHIEVEMENTS", 50, Color::YELLOW + Color::BOLD);
    printLine('=', 50, Color::YELLOW);

    int unlocked = 0;
    int total = 9;
    
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
    showAch(achPlinkoJackpot, "Plinko Jackpot", "Land in a Plinko edge bucket");
    
    printLine('-', 50, Color::DIM);
    cout << "\n  " << Color::WHITE << "Progress: " << Color::YELLOW 
         << unlocked << "/" << total << Color::RESET << " achievements unlocked\n";
    
    cout << "\n  Press any key to continue...";
    getch();
}

/*
 * displayGameOver() - Show game over screen when player goes bankrupt
 */
void GamblingGame::displayGameOver() {
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
void GamblingGame::displayExitScreen() {
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
