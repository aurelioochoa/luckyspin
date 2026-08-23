/*
 * menu.h - Main menu table.
 *
 * The menu is a single table. Previously three separate things had to be
 * kept in sync by hand - the MENU_ITEMS constant, the local items[] array
 * in displayMenu() and the switch in play(). Now the table alone drives
 * rendering, navigation and dispatch.
 */

#ifndef MENU_H
#define MENU_H

#include <string>

#include "symbols.h" // UI:: icons

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
    SpinWheel, PlaySlots, Blackjack, Roulette, Plinko,   // GAMES
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
    {nullptr,   UI::TARGET,  "Plinko",          MenuAction::Plinko},
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

#endif // MENU_H
