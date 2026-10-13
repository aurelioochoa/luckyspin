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
#include "core/terminal_ui.h"
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
namespace {
void dashboardLine(const string& content, int width) {
    cout << "  " << tui::fit(content, width) << Color::RESET << "\n";
}
void dashboardRule(int width) {
    cout << "  " << Color::DIM;
    for (int i = 0; i < width; ++i) cout << "─";
    cout << Color::RESET << "\n";
}
}

void GamblingGame::displayHeader() {
    const int width = tui::dashboardWidth(terminal::columns());
    dashboardLine(Color::YELLOW + Color::BOLD + "♠  LUCKY SPIN" + Color::RESET
                  + Color::DIM + "  /  casino & idle empire", width);
    dashboardRule(width);
}

/*
 * showMilestone() - Display milestone unlock notification (Phase C)
 */
void GamblingGame::showMilestone(const string& name, const string& desc) {
    notificationQueue.push_back("milestone|" + name + "|" + desc);
    if (headlessMode) return;
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
    const int width = tui::dashboardWidth(terminal::columns());
    const string balance = Color::BOLD + Color::GREEN + formatMoney(money) + Color::RESET;
    const string income = Color::GREEN + "+" + formatMoney(getTotalPassiveIncome()) + "/s" + Color::RESET;
    if (width >= 68) {
        const int half = width / 2;
        dashboardLine(tui::fit("BANKROLL  " + balance, half) + "PASSIVE  " + income, width);
        dashboardLine(tui::fit("Bet " + formatMoney(betAmount) + "  ·  Round " + to_string(round), half)
                      + "Prestige " + to_string(prestigeLevel) + "  ·  Stars " + to_string(prestigeStars), width);
        dashboardLine(tui::fit("Wins " + to_string(totalWins) + "  /  Losses " + to_string(totalLosses), half)
                      + "Peak " + formatMoney(peakMoney), width);
    } else {
        dashboardLine("BANKROLL " + balance + "  ·  Bet " + formatMoney(betAmount), width);
        dashboardLine("Income " + income + "  ·  P" + to_string(prestigeLevel), width);
        dashboardLine("Round " + to_string(round) + "  ·  W " + to_string(totalWins)
                      + " / L " + to_string(totalLosses), width);
    }
    if (activeEventId != economy::EVENT_NONE) {
        dashboardLine(Color::YELLOW + string("Event: ") + economy::eventName(activeEventId)
                      + " · " + to_string(std::max(0L, (long)(eventEndTime - time(0)))) + "s left", width);
    } else {
        dashboardLine(Color::DIM + "Build your income. Upgrade your odds. Play your way.", width);
    }
    dashboardRule(width);
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
        case MenuAction::About:           return "(rules and keyboard controls)";
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
    const int width = tui::dashboardWidth(terminal::columns());
    auto entry = [&](int index, int cellWidth) {
        const MenuEntry& item = MENU_TABLE[index];
        const bool selected = index == menuSelection;
        const string prefix = selected ? Color::BG_GREEN + Color::WHITE + Color::BOLD : Color::WHITE;
        return prefix + tui::fit(string(selected ? "› " : "  ") + tui::shortcut(index)
                    + "  " + item.name, cellWidth) + Color::RESET;
    };
    if (width >= 68 && terminal::rows() >= 23) {
        const int half = (width - 3) / 2;
        dashboardLine(Color::YELLOW + tui::fit("THE FLOOR / SHOPS", half) + "   YOUR EMPIRE", width);
        for (int row = 0; row < 8; ++row) {
            const string left = row < 7 ? entry(row, half) : tui::fit("", half);
            dashboardLine(left + "   " + entry(row + 7, width - half - 3), width);
        }
    } else {
        // Keep the highlighted item visible on short or narrow terminals.
        const int visible = std::max(3, std::min(MENU_ITEM_COUNT, terminal::rows() - 14));
        const int first = std::max(0, std::min(menuSelection - visible / 2, MENU_ITEM_COUNT - visible));
        for (int i = first; i < first + visible; ++i) dashboardLine(entry(i, width), width);
        if (visible < MENU_ITEM_COUNT)
            dashboardLine(Color::DIM + "More choices ↑/↓  ·  " + to_string(menuSelection + 1)
                          + "/" + to_string(MENU_ITEM_COUNT), width);
    }
    dashboardRule(width);
    dashboardLine(Color::YELLOW + MENU_TABLE[menuSelection].name + Color::RESET
                  + " " + menuDesc(MENU_TABLE[menuSelection].action), width);
    dashboardLine(Color::DIM + "↑/↓ W/S move · Enter play · 0–9 jump", width);
    dashboardLine(Color::DIM + "V save · ? help · Q quit", width);
}

/*
 * displayStatus() - Main display function combining all elements
 */
void GamblingGame::displayStatus() {
    cout << "\033[H";
    displayHeader();
    displayStats();
    displayMenu();
    cout << "\033[J" << flush;
}

void GamblingGame::displayAbout() {
    clearScreen();
    displayHeader();
    const int width = tui::dashboardWidth(terminal::columns());
    const vector<string> paragraphs = {
        "ABOUT LUCKY SPIN", "A local C++ casino & incremental game. MIT license.",
        "All balances are fictional game currency.", "",
        "START HERE", "Buy a $10 Tip Jar in Generator Shop for $0.10/s.",
        "Income grows while you play; offline earnings apply on return.",
        "Upgrade games, combine generators, then prestige for stars.", "",
        "THE TABLES", "Wheel: base 50% chance, 2x gross return.",
        "Slots: match three symbols for a 5x base return.",
        "Blackjack: reach 21; hit draws, stand settles.",
        "Roulette: red/black, odd/even, or a number from 0–36.",
        "Plinko: choose risk, drop a ball; edges pay up to 35x.", "",
        "KEYS", "Arrows / W/S: move. Enter / Space / D: select.",
        "0–9: quick actions. V: save. Q: quit. ?: this help.",
        "Restart clears ALL progress, including prestige and perks.",
        "Auto-save: every 30 seconds. Quit saves before leaving."
    };
    vector<string> lines;
    for (const string& paragraph : paragraphs) {
        const vector<string> wrapped = tui::wrap(paragraph, width);
        lines.insert(lines.end(), wrapped.begin(), wrapped.end());
    }
    const int pageSize = std::max(3, terminal::rows() - 6);
    for (size_t start = 0; start < lines.size(); start += pageSize) {
        if (start) { clearScreen(); displayHeader(); }
        for (size_t i = start; i < lines.size() && i < start + pageSize; ++i)
            dashboardLine(lines[i], width);
        dashboardRule(width);
        dashboardLine(Color::YELLOW + string(start + pageSize < lines.size()
                      ? "Any key: next page · Q: return" : "Any key: return to the floor"), width);
        char key = getch();
        if (key == 'q' || key == 'Q') break;
    }
}

// ========================================================================
// ACHIEVEMENT SYSTEM (Sprint 4 - Polish)
// ========================================================================

/*
 * showAchievement() - Display achievement unlock notification
 *
 * Also queued for the web GUI, which renders it as a toast on its next poll.
 */
void GamblingGame::showAchievement(const string& name, const string& desc) {
    notificationQueue.push_back("achievement|" + name + "|" + desc);
    if (headlessMode) return;
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
    notificationQueue.push_back("event|" + name + "|" + desc);
    if (headlessMode) return;
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
    const string names[] = {"First Win", "High Roller", "Millionaire", "Lucky Streak", "Generator Tycoon", "Fully Upgraded", "First Prestige", "Prestige Master", "Plinko Jackpot"};
    const string descriptions[] = {"Win your first game", "Win $1,000 in one bet", "Reach $1,000,000", "Win 10 games in a row", "Own 10 of any generator", "Max every upgrade", "Prestige for the first time", "Reach prestige level 5", "Land in a Plinko edge bucket"};
    int selected = 0;
    while (true) {
        updatePassiveIncome(); autoSave();
        cout << "\033[H"; displayHeader();
        terminal::line(Color::YELLOW + Color::BOLD + "YOUR COLLECTION");
        terminal::line(to_string(getAchievementCount()) + "/9 unlocked · +1% income each");
        terminal::rule();
        const int visible = std::max(3, std::min(9, terminal::rows() - 10));
        const int first = std::max(0, std::min(selected - visible / 2, 9 - visible));
        for (int i = first; i < first + visible; ++i) {
            const string style = i == selected ? Color::BG_GREEN + Color::WHITE + Color::BOLD : (getAchievementEarned(i) ? Color::YELLOW : Color::DIM);
            terminal::line(style + string(i == selected ? "› " : "  ") + (getAchievementEarned(i) ? "★ " : "☆ ") + names[i]);
        }
        terminal::rule();
        terminal::line(Color::DIM + descriptions[selected]);
        terminal::line(Color::DIM + "↑/↓ W/S browse · Q / Enter back");
        terminal::finishScreen();
        const char key = getMenuInput(1000);
        if (key == 'w') selected = (selected + 8) % 9;
        else if (key == 's') selected = (selected + 1) % 9;
        else if (key == 'q' || key == 'Q' || key == 'a' || key == '\n' || key == '\r') return;
    }
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
