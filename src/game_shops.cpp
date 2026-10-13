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
void GamblingGame::displayUpgradeShop() { displayShop(false); }

void GamblingGame::displayShop(bool generatorShop) {
    const int count = generatorShop ? generators.size() : upgrades.size();
    int selected = 1, mode = 0;
    const int amounts[] = {1, 10, 25, 100, 0};
    const string modes[] = {"x1", "x10", "x25", "x100", "MAX"};
    string feedback;
    while (true) {
        updatePassiveIncome();
        autoSave();
        cout << "\033[H";
        displayHeader();
        terminal::line(Color::YELLOW + Color::BOLD + (generatorShop ? "BUILD THE HOUSE / GENERATORS" : "STACK THE ODDS / UPGRADES"));
        terminal::line("Balance " + formatMoney(money) + "  ·  Income +" + formatMoney(getTotalPassiveIncome()) + "/s");
        terminal::line(generatorShop ? "Buy quantity " + modes[mode] + "  ·  E cycles quantity" : "Choose a game improvement for this run.");
        terminal::rule();
        const int visible = std::max(3, std::min(count + 1, terminal::rows() - 13));
        const int first = std::max(0, std::min(selected - visible / 2, count + 1 - visible));
        for (int row = first; row < first + visible; ++row) {
            string label = "Back to the floor";
            if (row > 0) {
                if (generatorShop) {
                    const Generator& g = generators[row - 1];
                    label = g.getName() + "  ×" + to_string(g.getOwned());
                } else {
                    const Upgrade& u = upgrades[row - 1];
                    label = u.getName() + "  Lv." + to_string(u.getCurrentLevel()) + "/" + to_string(u.getMaxLevel());
                }
            }
            const string style = row == selected ? Color::BG_GREEN + Color::BOLD + Color::WHITE : Color::WHITE;
            terminal::line(style + string(row == selected ? "› " : "  ") + (row < 10 ? to_string(row) : " ") + "  " + label);
        }
        terminal::rule();
        if (selected > 0) {
            if (generatorShop) {
                const Generator& g = generators[selected - 1];
                const int amount = amounts[mode] == 0 ? getGeneratorMaxAffordable(g, money) : amounts[mode];
                const double cost = getGeneratorBulkCost(g, std::max(1, amount));
                terminal::line((money >= cost && amount > 0 ? Color::GREEN : Color::RED)
                              + "Buy ×" + to_string(amount) + " for " + formatMoney(cost));
                terminal::line(Color::DIM + g.getDescription());
            } else {
                const Upgrade& u = upgrades[selected - 1];
                terminal::line(u.isMaxed() ? Color::YELLOW + "Fully upgraded" :
                    (money >= u.getCurrentCost() ? Color::GREEN : Color::RED) + "Next level: " + formatMoney(u.getCurrentCost()));
                terminal::line(Color::DIM + u.getDescription());
            }
        } else {
            terminal::line(Color::DIM + "Your casino keeps earning while you browse.");
            terminal::line("");
        }
        terminal::line(feedback.empty() ? Color::DIM + "↑/↓ W/S move · Enter buy · 1–9 buy" : Color::YELLOW + feedback);
        terminal::line(Color::DIM + "Q / A / 0 back  ·  " + to_string(selected) + "/" + to_string(count));
        terminal::finishScreen();
        const char key = getMenuInput(1000);
        if (!key) continue;
        if (key == 'q' || key == 'Q' || key == 'a' || key == '0') return;
        if (key == 'w') { selected = (selected + count) % (count + 1); feedback.clear(); continue; }
        if (key == 's') { selected = (selected + 1) % (count + 1); feedback.clear(); continue; }
        if ((key == 'e' || key == 'E') && generatorShop) { mode = (mode + 1) % 5; continue; }
        if (key >= '1' && key <= '9' && key - '0' <= count) selected = key - '0';
        else if (key != '\n' && key != '\r' && key != ' ' && key != 'd') continue;
        if (selected == 0) return;
        double cost = 0;
        if (generatorShop) {
            int bought = 0;
            if (buyGeneratorCount(selected - 1, amounts[mode], cost, bought)) {
                feedback = "Bought ×" + to_string(bought) + " · " + formatMoney(cost);
                saveGame();
            } else feedback = "Need " + formatMoney(cost) + " for this purchase.";
        } else {
            if (buyUpgradeLevel(selected - 1, cost)) { feedback = "Upgrade purchased · " + formatMoney(cost); saveGame(); }
            else feedback = upgrades[selected - 1].isMaxed() ? "Already fully upgraded." : "Need " + formatMoney(cost) + ".";
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
    while (true) {
        updatePassiveIncome(); autoSave();
        cout << "\033[H"; displayHeader();
        terminal::line(Color::YELLOW + Color::BOLD + "A FRESH START / PRESTIGE");
        terminal::line("Level " + to_string(prestigeLevel) + " · Stars " + to_string(prestigeStars));
        terminal::rule();
        terminal::line("This run: " + formatMoney(lifetimeEarnings));
        terminal::line("Target:   " + formatMoney(getPrestigeRequirement()));
        const int length = std::max(10, std::min(40, tui::dashboardWidth(terminal::columns()) - 8));
        const double progress = std::min(1.0, lifetimeEarnings / getPrestigeRequirement());
        string bar;
        for (int i = 0; i < length; ++i) bar += i < (int)(length * progress) ? "━" : "·";
        terminal::line(Color::GREEN + bar + " " + to_string((int)(progress * 100)) + "%");
        terminal::line(Color::DIM + "Reset this run for +10% income and stars.");
        terminal::line(Color::DIM + "Achievements and perks carry forward.");
        terminal::rule();
        terminal::line((canPrestige() ? Color::GREEN : Color::DIM) + string("P  Prestige ") + (canPrestige() ? "available" : "locked"));
        terminal::line(Color::YELLOW + "S  Spend stars on permanent perks");
        terminal::line(Color::DIM + "Q / 0 back to the floor");
        terminal::finishScreen();
        const char key = getchWithTimeout(1000);
        if ((key == 'p' || key == 'P') && canPrestige()) performPrestige();
        else if (key == 's' || key == 'S') displayPrestigeShop();
        else if (key == 'q' || key == 'Q' || key == '0') return;
    }
}

/*
 * displayPrestigeShop() - Show prestige shop with permanent perks (Phase B)
 */
void GamblingGame::displayPrestigeShop() {
    const string names[] = {"Starting Money", "Generator Discount", "Offline Cap", "Passive Boost"};
    const string descriptions[] = {"+$50 starting money per level", "5% cheaper generators per level", "+2h offline earnings cap per level", "+5% passive income per level"};
    const int maximum[] = {10, 10, 5, 20}, base[] = {3, 5, 8, 4};
    int selected = 0;
    string feedback;
    while (true) {
        updatePassiveIncome(); autoSave();
        cout << "\033[H"; displayHeader();
        terminal::line(Color::YELLOW + Color::BOLD + "PERMANENT PERKS");
        terminal::line("Available stars: " + to_string(prestigeStars) + " ★");
        terminal::rule();
        for (int i = 0; i < 4; ++i) {
            const int level = getPerkLevel(i);
            const string style = i == selected ? Color::BG_GREEN + Color::WHITE + Color::BOLD : Color::WHITE;
            terminal::line(style + string(i == selected ? "› " : "  ") + names[i] + " " + to_string(level) + "/" + to_string(maximum[i]));
        }
        terminal::rule();
        const int level = getPerkLevel(selected);
        terminal::line(level == maximum[selected] ? Color::YELLOW + "Fully upgraded" : "Next level: " + to_string(base[selected] + level * 2) + " ★");
        terminal::line(Color::DIM + descriptions[selected]);
        terminal::line(Color::YELLOW + feedback);
        terminal::line(Color::DIM + "↑/↓ W/S move · Enter buy · Q back");
        terminal::finishScreen();
        const char key = getMenuInput(1000);
        if (key == 'q' || key == 'Q' || key == 'a') return;
        if (key == 'w') { selected = (selected + 3) % 4; continue; }
        if (key == 's') { selected = (selected + 1) % 4; continue; }
        if (key == '\n' || key == '\r' || key == ' ' || key == 'd') {
            int cost = 0;
            if (buyPerkLevel(selected, cost)) { feedback = "Perk purchased · " + to_string(cost) + " ★"; saveGame(); }
            else feedback = level == maximum[selected] ? "Already fully upgraded." : "You need more stars.";
        }
    }
}

/*
 * displaySpecializationMenu() - Choose a specialization build (Phase C)
 */
void GamblingGame::displaySpecializationMenu() {
    const string names[] = {"All-rounder", "Wheel Tycoon", "Slot Mogul", "Card Shark", "Roulette Baron", "Plinko Pro"};
    int selected = specializationId;
    while (true) {
        updatePassiveIncome(); autoSave();
        cout << "\033[H";
        displayHeader();
        terminal::line(Color::YELLOW + Color::BOLD + "CHOOSE YOUR SPECIALTY");
        terminal::line(Color::DIM + "+50% return at your chosen table.");
        terminal::rule();
        for (int i = 0; i < 6; ++i) {
            const string style = i == selected ? Color::BG_GREEN + Color::BOLD + Color::WHITE : Color::WHITE;
            terminal::line(style + string(i == selected ? "› " : "  ") + to_string(i) + "  " + names[i]
                          + (i == specializationId ? "  ✓" : ""));
        }
        terminal::rule();
        terminal::line(Color::DIM + "Free to change · All-rounder has no bonus.");
        terminal::line(Color::DIM + "↑/↓ move · Enter choose · 0–5 choose · Q back");
        terminal::finishScreen();
        const char key = getMenuInput(1000);
        if (!key) continue;
        if (key == 'q' || key == 'Q' || key == 'a') return;
        if (key == 'w') { selected = (selected + 5) % 6; continue; }
        if (key == 's') { selected = (selected + 1) % 6; continue; }
        if (key >= '0' && key <= '5') selected = key - '0';
        else if (key != '\n' && key != '\r' && key != ' ' && key != 'd') continue;
        setSpecializationId(selected); saveGame(); return;
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
void GamblingGame::displayGeneratorShop() { displayShop(true); }

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
