/*
 * Unicode Symbol Compatibility Test
 * 
 * This file tests all Unicode symbols used in the Lucky Spin Casino game.
 * Run this to verify your terminal supports the required characters.
 * 
 * Compile: g++ -std=c++11 -o test_unicode test_unicode.cpp
 * Run: ./test_unicode
 */

#include <iostream>
#include <string>

using namespace std;

// ANSI Color codes for formatting
namespace Color {
    const string RESET   = "\033[0m";
    const string BOLD    = "\033[1m";
    const string DIM     = "\033[2m";
    const string RED     = "\033[31m";
    const string GREEN   = "\033[32m";
    const string YELLOW  = "\033[33m";
    const string BLUE    = "\033[34m";
    const string MAGENTA = "\033[35m";
    const string CYAN    = "\033[36m";
    const string WHITE   = "\033[37m";
}

int main() {
    cout << Color::BOLD << Color::CYAN;
    cout << "╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║           UNICODE SYMBOL COMPATIBILITY TEST                  ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n";
    cout << Color::RESET << "\n";

    cout << "If symbols appear as boxes, question marks, or garbled text,\n";
    cout << "your terminal may not support those Unicode characters.\n\n";

    // ========================================================================
    // BOX DRAWING CHARACTERS (U+2500 block)
    // ========================================================================
    cout << Color::BOLD << Color::YELLOW << "═══ BOX DRAWING CHARACTERS ═══" << Color::RESET << "\n\n";
    
    cout << "  Light box drawing:\n";
    cout << "    Horizontal:     " << "\xE2\x94\x80" << "  (U+2500 ─)\n";
    cout << "    Vertical:       " << "\xE2\x94\x82" << "  (U+2502 │)\n";
    cout << "    Top-left:       " << "\xE2\x94\x8C" << "  (U+250C ┌)\n";
    cout << "    Top-right:      " << "\xE2\x94\x90" << "  (U+2510 ┐)\n";
    cout << "    Bottom-left:    " << "\xE2\x94\x94" << "  (U+2514 └)\n";
    cout << "    Bottom-right:   " << "\xE2\x94\x98" << "  (U+2518 ┘)\n";
    cout << "    T-down:         " << "\xE2\x94\xAC" << "  (U+252C ┬)\n";
    cout << "    T-up:           " << "\xE2\x94\xB4" << "  (U+2534 ┴)\n";
    cout << "    T-right:        " << "\xE2\x94\x9C" << "  (U+251C ├)\n";
    cout << "    T-left:         " << "\xE2\x94\xA4" << "  (U+2524 ┤)\n";
    cout << "    Cross:          " << "\xE2\x94\xBC" << "  (U+253C ┼)\n";
    
    cout << "\n  Double box drawing:\n";
    cout << "    Horizontal:     " << "\xE2\x95\x90" << "  (U+2550 ═)\n";
    cout << "    Vertical:       " << "\xE2\x95\x91" << "  (U+2551 ║)\n";
    cout << "    Top-left:       " << "\xE2\x95\x94" << "  (U+2554 ╔)\n";
    cout << "    Top-right:      " << "\xE2\x95\x97" << "  (U+2557 ╗)\n";
    cout << "    Bottom-left:    " << "\xE2\x95\x9A" << "  (U+255A ╚)\n";
    cout << "    Bottom-right:   " << "\xE2\x95\x9D" << "  (U+255D ╝)\n";

    cout << "\n  Sample box:\n";
    cout << "    \xE2\x94\x8C\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x90\n";
    cout << "    \xE2\x94\x82 Test \xE2\x94\x82\n";
    cout << "    \xE2\x94\x94\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80\xE2\x94\x98\n";

    // ========================================================================
    // CARD SUITS (U+2660 block)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CARD SUITS ═══" << Color::RESET << "\n\n";
    
    cout << "  Black suits:\n";
    cout << "    Spade:          " << "\xE2\x99\xA0" << "  (U+2660 ♠)\n";
    cout << "    Heart:          " << Color::RED << "\xE2\x99\xA5" << Color::RESET << "  (U+2665 ♥)\n";
    cout << "    Diamond:        " << Color::RED << "\xE2\x99\xA6" << Color::RESET << "  (U+2666 ♦)\n";
    cout << "    Club:           " << "\xE2\x99\xA3" << "  (U+2663 ♣)\n";
    
    cout << "\n  White suits:\n";
    cout << "    Spade:          " << "\xE2\x99\xA4" << "  (U+2664 ♤)\n";
    cout << "    Heart:          " << "\xE2\x99\xA1" << "  (U+2661 ♡)\n";
    cout << "    Diamond:        " << "\xE2\x99\xA2" << "  (U+2662 ♢)\n";
    cout << "    Club:           " << "\xE2\x99\xA7" << "  (U+2667 ♧)\n";

    // ========================================================================
    // STARS (U+2605 block and others)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ STARS ═══" << Color::RESET << "\n\n";
    
    cout << "    Black star:     " << Color::YELLOW << "\xE2\x98\x85" << Color::RESET << "  (U+2605 ★)\n";
    cout << "    White star:     " << Color::YELLOW << "\xE2\x98\x86" << Color::RESET << "  (U+2606 ☆)\n";
    cout << "    Medium star:    " << Color::YELLOW << "\xE2\xAD\x90" << Color::RESET << "  (U+2B50 ⭐)\n";
    cout << "    Sparkle:        " << Color::YELLOW << "\xE2\x9C\xA8" << Color::RESET << "  (U+2728 ✨)\n";

    // ========================================================================
    // CHECK MARKS AND CROSSES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CHECK MARKS & CROSSES ═══" << Color::RESET << "\n\n";
    
    cout << "    Check mark:     " << Color::GREEN << "\xE2\x9C\x93" << Color::RESET << "  (U+2713 ✓)\n";
    cout << "    Heavy check:    " << Color::GREEN << "\xE2\x9C\x94" << Color::RESET << "  (U+2714 ✔)\n";
    cout << "    Ballot X:       " << Color::RED << "\xE2\x9C\x97" << Color::RESET << "  (U+2717 ✗)\n";
    cout << "    Heavy X:        " << Color::RED << "\xE2\x9C\x98" << Color::RESET << "  (U+2718 ✘)\n";
    cout << "    Cross mark:     " << Color::RED << "\xE2\x9D\x8C" << Color::RESET << "  (U+274C ❌)\n";
    cout << "    Green check:    " << Color::GREEN << "\xE2\x9C\x85" << Color::RESET << "  (U+2705 ✅)\n";

    // ========================================================================
    // ARROWS AND TRIANGLES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ ARROWS & TRIANGLES ═══" << Color::RESET << "\n\n";
    
    cout << "  Triangles:\n";
    cout << "    Right:          " << "\xE2\x96\xB6" << "  (U+25B6 ▶)\n";
    cout << "    Left:           " << "\xE2\x97\x80" << "  (U+25C0 ◀)\n";
    cout << "    Up:             " << "\xE2\x96\xB2" << "  (U+25B2 ▲)\n";
    cout << "    Down:           " << "\xE2\x96\xBC" << "  (U+25BC ▼)\n";
    
    cout << "\n  Arrows:\n";
    cout << "    Up:             " << "\xE2\xAC\x86" << "  (U+2B06 ⬆)\n";
    cout << "    Down:           " << "\xE2\xAC\x87" << "  (U+2B07 ⬇)\n";
    cout << "    Left:           " << "\xE2\xAC\x85" << "  (U+2B05 ⬅)\n";
    cout << "    Right:          " << "\xE2\x9E\xA1" << "  (U+27A1 ➡)\n";
    cout << "    Refresh:        " << "\xF0\x9F\x94\x84" << "  (U+1F504 🔄)\n";

    // ========================================================================
    // SLOT MACHINE SYMBOLS (Emoji - 4-byte UTF-8)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ SLOT MACHINE SYMBOLS (Emoji) ═══" << Color::RESET << "\n\n";
    
    cout << "  Fruits:\n";
    cout << "    Cherry:         " << Color::RED << "\xF0\x9F\x8D\x92" << Color::RESET << "  (U+1F352 🍒)\n";
    cout << "    Tangerine:      " << Color::YELLOW << "\xF0\x9F\x8D\x8A" << Color::RESET << "  (U+1F34A 🍊)\n";
    cout << "    Lemon:          " << Color::YELLOW << "\xF0\x9F\x8D\x8B" << Color::RESET << "  (U+1F34B 🍋)\n";
    cout << "    Grapes:         " << Color::MAGENTA << "\xF0\x9F\x8D\x87" << Color::RESET << "  (U+1F347 🍇)\n";
    cout << "    Watermelon:     " << Color::GREEN << "\xF0\x9F\x8D\x89" << Color::RESET << "  (U+1F349 🍉)\n";
    cout << "    Banana:         " << Color::YELLOW << "\xF0\x9F\x8D\x8C" << Color::RESET << "  (U+1F34C 🍌)\n";
    
    cout << "\n  Casino:\n";
    cout << "    Bell:           " << Color::YELLOW << "\xF0\x9F\x94\x94" << Color::RESET << "  (U+1F514 🔔)\n";
    cout << "    Gem:            " << Color::CYAN << "\xF0\x9F\x92\x8E" << Color::RESET << "  (U+1F48E 💎)\n";
    cout << "    Slot machine:   " << "\xF0\x9F\x8E\xB0" << "  (U+1F3B0 🎰)\n";
    cout << "    Game die:       " << "\xF0\x9F\x8E\xB2" << "  (U+1F3B2 🎲)\n";
    cout << "    Playing card:   " << "\xF0\x9F\x83\x8F" << "  (U+1F0CF 🃏)\n";
    cout << "    Ferris wheel:   " << "\xF0\x9F\x8E\xA1" << "  (U+1F3A1 🎡)\n";
    
    cout << "\n  Keycap Seven:\n";
    cout << "    7 keycap:       " << "7\xEF\xB8\x8F\xE2\x83\xA3" << "  (7️⃣)\n";

    // ========================================================================
    // MONEY AND BUSINESS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ MONEY & BUSINESS ═══" << Color::RESET << "\n\n";
    
    cout << "    Money bag:      " << Color::YELLOW << "\xF0\x9F\x92\xB0" << Color::RESET << "  (U+1F4B0 💰)\n";
    cout << "    Dollar:         " << Color::GREEN << "\xF0\x9F\x92\xB5" << Color::RESET << "  (U+1F4B5 💵)\n";
    cout << "    Euro:           " << "\xF0\x9F\x92\xB6" << "  (U+1F4B6 💶)\n";
    cout << "    Yen:            " << "\xF0\x9F\x92\xB4" << "  (U+1F4B4 💴)\n";
    cout << "    Pound:          " << "\xF0\x9F\x92\xB7" << "  (U+1F4B7 💷)\n";
    cout << "    Chart up:       " << Color::GREEN << "\xF0\x9F\x93\x88" << Color::RESET << "  (U+1F4C8 📈)\n";
    cout << "    Chart down:     " << Color::RED << "\xF0\x9F\x93\x89" << Color::RESET << "  (U+1F4C9 📉)\n";
    cout << "    Trophy:         " << Color::YELLOW << "\xF0\x9F\x8F\x86" << Color::RESET << "  (U+1F3C6 🏆)\n";
    cout << "    Crown:          " << Color::YELLOW << "\xF0\x9F\x91\x91" << Color::RESET << "  (U+1F451 👑)\n";

    // ========================================================================
    // UI ICONS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ UI ICONS ═══" << Color::RESET << "\n\n";
    
    cout << "    Store:          " << "\xF0\x9F\x8F\xAA" << "  (U+1F3EA 🏪)\n";
    cout << "    Building:       " << "\xF0\x9F\x8F\x9B" << "  (U+1F3DB 🏛)\n";
    cout << "    Door:           " << "\xF0\x9F\x9A\xAA" << "  (U+1F6AA 🚪)\n";
    cout << "    Floppy disk:    " << "\xF0\x9F\x92\xBE" << "  (U+1F4BE 💾)\n";
    cout << "    Fire:           " << Color::RED << "\xF0\x9F\x94\xA5" << Color::RESET << "  (U+1F525 🔥)\n";
    cout << "    Alarm clock:    " << "\xE2\x8F\xB0" << "  (U+23F0 ⏰)\n";
    cout << "    Globe:          " << "\xF0\x9F\x8C\x8D" << "  (U+1F30D 🌍)\n";
    cout << "    Jar:            " << "\xF0\x9F\xAB\x99" << "  (U+1FAD9 🫙)\n";
    cout << "    Gear:           " << "\xE2\x9A\x99" << "  (U+2699 ⚙)\n";
    cout << "    Warning:        " << Color::YELLOW << "\xE2\x9A\xA0" << Color::RESET << "  (U+26A0 ⚠)\n";
    cout << "    Info:           " << Color::BLUE << "\xE2\x84\xB9" << Color::RESET << "  (U+2139 ℹ)\n";

    // ========================================================================
    // MISCELLANEOUS SYMBOLS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ MISCELLANEOUS ═══" << Color::RESET << "\n\n";
    
    cout << "    Bullet:         " << "\xE2\x80\xA2" << "  (U+2022 •)\n";
    cout << "    Circle:         " << "\xE2\x97\x8F" << "  (U+25CF ●)\n";
    cout << "    Empty circle:   " << "\xE2\x97\x8B" << "  (U+25CB ○)\n";
    cout << "    Square:         " << "\xE2\x96\xA0" << "  (U+25A0 ■)\n";
    cout << "    Empty square:   " << "\xE2\x96\xA1" << "  (U+25A1 □)\n";
    cout << "    Diamond:        " << "\xE2\x97\x86" << "  (U+25C6 ◆)\n";
    cout << "    Infinity:       " << "\xE2\x88\x9E" << "  (U+221E ∞)\n";
    cout << "    Ellipsis:       " << "\xE2\x80\xA6" << "  (U+2026 …)\n";
    cout << "    Section:        " << "\xC2\xA7" << "  (U+00A7 §)\n";
    cout << "    Degree:         " << "\xC2\xB0" << "  (U+00B0 °)\n";
    cout << "    Copyright:      " << "\xC2\xA9" << "  (U+00A9 ©)\n";

    // ========================================================================
    // BLOCK ELEMENTS (for progress bars)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ BLOCK ELEMENTS (Progress Bars) ═══" << Color::RESET << "\n\n";
    
    cout << "    Full block:     " << "\xE2\x96\x88" << "  (U+2588 █)\n";
    cout << "    Light shade:    " << "\xE2\x96\x91" << "  (U+2591 ░)\n";
    cout << "    Medium shade:   " << "\xE2\x96\x92" << "  (U+2592 ▒)\n";
    cout << "    Dark shade:     " << "\xE2\x96\x93" << "  (U+2593 ▓)\n";
    cout << "    Left half:      " << "\xE2\x96\x8C" << "  (U+258C ▌)\n";
    cout << "    Right half:     " << "\xE2\x96\x90" << "  (U+2590 ▐)\n";
    
    cout << "\n  Sample progress bar:\n";
    cout << "    [" << Color::GREEN;
    for (int i = 0; i < 10; i++) cout << "\xE2\x96\x88";
    cout << Color::DIM;
    for (int i = 0; i < 10; i++) cout << "\xE2\x96\x91";
    cout << Color::RESET << "] 50%\n";

    // ========================================================================
    // FACES AND EXPRESSIONS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ FACES & EXPRESSIONS ═══" << Color::RESET << "\n\n";
    
    cout << "    Grinning:       " << "\xF0\x9F\x98\x80" << "  (U+1F600 😀)\n";
    cout << "    Money face:     " << "\xF0\x9F\xA4\x91" << "  (U+1F911 🤑)\n";
    cout << "    Thinking:       " << "\xF0\x9F\xA4\x94" << "  (U+1F914 🤔)\n";
    cout << "    Sunglasses:     " << "\xF0\x9F\x98\x8E" << "  (U+1F60E 😎)\n";
    cout << "    Crying:         " << "\xF0\x9F\x98\xA2" << "  (U+1F622 😢)\n";
    cout << "    Skull:          " << "\xF0\x9F\x92\x80" << "  (U+1F480 💀)\n";
    cout << "    Party:          " << "\xF0\x9F\xA5\xB3" << "  (U+1F973 🥳)\n";
    cout << "    Thumbs up:      " << "\xF0\x9F\x91\x8D" << "  (U+1F44D 👍)\n";
    cout << "    Thumbs down:    " << "\xF0\x9F\x91\x8E" << "  (U+1F44E 👎)\n";
    cout << "    Clap:           " << "\xF0\x9F\x91\x8F" << "  (U+1F44F 👏)\n";

    // ========================================================================
    // DICE (for dice games)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ DICE ═══" << Color::RESET << "\n\n";
    
    cout << "    Die face 1:     " << "\xE2\x9A\x80" << "  (U+2680 ⚀)\n";
    cout << "    Die face 2:     " << "\xE2\x9A\x81" << "  (U+2681 ⚁)\n";
    cout << "    Die face 3:     " << "\xE2\x9A\x82" << "  (U+2682 ⚂)\n";
    cout << "    Die face 4:     " << "\xE2\x9A\x83" << "  (U+2683 ⚃)\n";
    cout << "    Die face 5:     " << "\xE2\x9A\x84" << "  (U+2684 ⚄)\n";
    cout << "    Die face 6:     " << "\xE2\x9A\x85" << "  (U+2685 ⚅)\n";

    // ========================================================================
    // PLAYING CARDS (individual cards)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ PLAYING CARDS ═══" << Color::RESET << "\n\n";
    
    cout << "  Card backs:\n";
    cout << "    Card back:      " << "\xF0\x9F\x82\xA0" << "  (U+1F0A0 🂠)\n";
    
    cout << "\n  Spades (A-K):\n";
    cout << "    Ace:  " << "\xF0\x9F\x82\xA1" << "  2: " << "\xF0\x9F\x82\xA2" << "  3: " << "\xF0\x9F\x82\xA3" << "  4: " << "\xF0\x9F\x82\xA4" << "  5: " << "\xF0\x9F\x82\xA5" << "\n";
    cout << "    6:    " << "\xF0\x9F\x82\xA6" << "  7: " << "\xF0\x9F\x82\xA7" << "  8: " << "\xF0\x9F\x82\xA8" << "  9: " << "\xF0\x9F\x82\xA9" << "  10:" << "\xF0\x9F\x82\xAA" << "\n";
    cout << "    J:    " << "\xF0\x9F\x82\xAB" << "  Q: " << "\xF0\x9F\x82\xAD" << "  K: " << "\xF0\x9F\x82\xAE" << "\n";
    
    cout << "\n  Hearts (A-K):\n";
    cout << "    Ace:  " << "\xF0\x9F\x82\xB1" << "  2: " << "\xF0\x9F\x82\xB2" << "  3: " << "\xF0\x9F\x82\xB3" << "  4: " << "\xF0\x9F\x82\xB4" << "  5: " << "\xF0\x9F\x82\xB5" << "\n";
    cout << "    6:    " << "\xF0\x9F\x82\xB6" << "  7: " << "\xF0\x9F\x82\xB7" << "  8: " << "\xF0\x9F\x82\xB8" << "  9: " << "\xF0\x9F\x82\xB9" << "  10:" << "\xF0\x9F\x82\xBA" << "\n";
    cout << "    J:    " << "\xF0\x9F\x82\xBB" << "  Q: " << "\xF0\x9F\x82\xBD" << "  K: " << "\xF0\x9F\x82\xBE" << "\n";

    // ========================================================================
    // WEATHER & NATURE (for themes/effects)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ WEATHER & NATURE ═══" << Color::RESET << "\n\n";
    
    cout << "    Sun:            " << Color::YELLOW << "\xE2\x98\x80" << Color::RESET << "  (U+2600 ☀)\n";
    cout << "    Cloud:          " << "\xE2\x98\x81" << "  (U+2601 ☁)\n";
    cout << "    Umbrella:       " << "\xE2\x98\x82" << "  (U+2602 ☂)\n";
    cout << "    Snowman:        " << "\xE2\x98\x83" << "  (U+2603 ☃)\n";
    cout << "    Comet:          " << "\xE2\x98\x84" << "  (U+2604 ☄)\n";
    cout << "    Lightning:      " << Color::YELLOW << "\xE2\x9A\xA1" << Color::RESET << "  (U+26A1 ⚡)\n";
    cout << "    Snowflake:      " << Color::CYAN << "\xE2\x9D\x84" << Color::RESET << "  (U+2744 ❄)\n";
    cout << "    Rainbow:        " << "\xF0\x9F\x8C\x88" << "  (U+1F308 🌈)\n";
    cout << "    Moon:           " << "\xF0\x9F\x8C\x99" << "  (U+1F319 🌙)\n";
    cout << "    Full moon:      " << "\xF0\x9F\x8C\x95" << "  (U+1F315 🌕)\n";
    cout << "    Four leaf:      " << Color::GREEN << "\xF0\x9F\x8D\x80" << Color::RESET << "  (U+1F340 🍀)\n";

    // ========================================================================
    // HEARTS & LOVE (for lives/health)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ HEARTS & HEALTH ═══" << Color::RESET << "\n\n";
    
    cout << "    Red heart:      " << Color::RED << "\xE2\x9D\xA4" << Color::RESET << "  (U+2764 ❤)\n";
    cout << "    Orange heart:   " << "\xF0\x9F\xA7\xA1" << "  (U+1F9E1 🧡)\n";
    cout << "    Yellow heart:   " << Color::YELLOW << "\xF0\x9F\x92\x9B" << Color::RESET << "  (U+1F49B 💛)\n";
    cout << "    Green heart:    " << Color::GREEN << "\xF0\x9F\x92\x9A" << Color::RESET << "  (U+1F49A 💚)\n";
    cout << "    Blue heart:     " << Color::BLUE << "\xF0\x9F\x92\x99" << Color::RESET << "  (U+1F499 💙)\n";
    cout << "    Purple heart:   " << Color::MAGENTA << "\xF0\x9F\x92\x9C" << Color::RESET << "  (U+1F49C 💜)\n";
    cout << "    Black heart:    " << "\xF0\x9F\x96\xA4" << "  (U+1F5A4 🖤)\n";
    cout << "    Broken heart:   " << Color::RED << "\xF0\x9F\x92\x94" << Color::RESET << "  (U+1F494 💔)\n";
    cout << "    Sparkling:      " << "\xF0\x9F\x92\x96" << "  (U+1F496 💖)\n";
    cout << "    Heart exclam:   " << Color::RED << "\xE2\x9D\xA3" << Color::RESET << "  (U+2763 ❣)\n";

    // ========================================================================
    // NUMBERS IN CIRCLES (for menu options)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CIRCLED NUMBERS ═══" << Color::RESET << "\n\n";
    
    cout << "  Filled circles:\n";
    cout << "    ";
    for (int i = 0; i <= 9; i++) {
        // U+2776 to U+277F (➊ to ➒) and U+24FF for 0
        if (i == 0) cout << "\xE2\x93\xBF ";  // ⓿
        else cout << "\xE2\x9D" << (char)(0xB5 + i) << " ";  // ➊➋➌➍➎➏➐➑➒
    }
    cout << "\n";
    
    cout << "\n  Empty circles:\n";
    cout << "    ";
    for (int i = 0; i <= 9; i++) {
        // U+2460 to U+2468 (① to ⑨) and U+24EA for 0
        if (i == 0) cout << "\xE2\x93\xAA ";  // ⓪
        else cout << "\xE2\x91" << (char)(0x9F + i) << " ";  // ①②③④⑤⑥⑦⑧⑨
    }
    cout << "\n";

    // ========================================================================
    // ZODIAC & ASTROLOGY (for themes)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ ZODIAC SIGNS ═══" << Color::RESET << "\n\n";
    
    cout << "    Aries:          " << "\xE2\x99\x88" << "  (U+2648 ♈)\n";
    cout << "    Taurus:         " << "\xE2\x99\x89" << "  (U+2649 ♉)\n";
    cout << "    Gemini:         " << "\xE2\x99\x8A" << "  (U+264A ♊)\n";
    cout << "    Cancer:         " << "\xE2\x99\x8B" << "  (U+264B ♋)\n";
    cout << "    Leo:            " << "\xE2\x99\x8C" << "  (U+264C ♌)\n";
    cout << "    Virgo:          " << "\xE2\x99\x8D" << "  (U+264D ♍)\n";
    cout << "    Libra:          " << "\xE2\x99\x8E" << "  (U+264E ♎)\n";
    cout << "    Scorpio:        " << "\xE2\x99\x8F" << "  (U+264F ♏)\n";
    cout << "    Sagittarius:    " << "\xE2\x99\x90" << "  (U+2650 ♐)\n";
    cout << "    Capricorn:      " << "\xE2\x99\x91" << "  (U+2651 ♑)\n";
    cout << "    Aquarius:       " << "\xE2\x99\x92" << "  (U+2652 ♒)\n";
    cout << "    Pisces:         " << "\xE2\x99\x93" << "  (U+2653 ♓)\n";

    // ========================================================================
    // CHESS PIECES (for strategy games)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CHESS PIECES ═══" << Color::RESET << "\n\n";
    
    cout << "  White pieces:\n";
    cout << "    King:   " << "\xE2\x99\x94" << "  Queen:  " << "\xE2\x99\x95" << "  Rook:   " << "\xE2\x99\x96" << "  Bishop: " << "\xE2\x99\x97" << "  Knight: " << "\xE2\x99\x98" << "  Pawn:   " << "\xE2\x99\x99" << "\n";
    cout << "\n  Black pieces:\n";
    cout << "    King:   " << "\xE2\x99\x9A" << "  Queen:  " << "\xE2\x99\x9B" << "  Rook:   " << "\xE2\x99\x9C" << "  Bishop: " << "\xE2\x99\x9D" << "  Knight: " << "\xE2\x99\x9E" << "  Pawn:   " << "\xE2\x99\x9F" << "\n";

    // ========================================================================
    // MUSIC & SOUND (for audio feedback)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ MUSIC & SOUND ═══" << Color::RESET << "\n\n";
    
    cout << "    Music note:     " << "\xE2\x99\xAA" << "  (U+266A ♪)\n";
    cout << "    Music notes:    " << "\xE2\x99\xAB" << "  (U+266B ♫)\n";
    cout << "    Flat:           " << "\xE2\x99\xAD" << "  (U+266D ♭)\n";
    cout << "    Natural:        " << "\xE2\x99\xAE" << "  (U+266E ♮)\n";
    cout << "    Sharp:          " << "\xE2\x99\xAF" << "  (U+266F ♯)\n";
    cout << "    Speaker:        " << "\xF0\x9F\x94\x8A" << "  (U+1F50A 🔊)\n";
    cout << "    Muted:          " << "\xF0\x9F\x94\x87" << "  (U+1F507 🔇)\n";
    cout << "    Bell:           " << "\xF0\x9F\x94\x94" << "  (U+1F514 🔔)\n";
    cout << "    No bell:        " << "\xF0\x9F\x94\x95" << "  (U+1F515 🔕)\n";

    // ========================================================================
    // KEYS & LOCKS (for unlockables)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ KEYS & LOCKS ═══" << Color::RESET << "\n\n";
    
    cout << "    Key:            " << Color::YELLOW << "\xF0\x9F\x94\x91" << Color::RESET << "  (U+1F511 🔑)\n";
    cout << "    Old key:        " << "\xF0\x9F\x97\x9D" << "  (U+1F5DD 🗝)\n";
    cout << "    Locked:         " << "\xF0\x9F\x94\x92" << "  (U+1F512 🔒)\n";
    cout << "    Unlocked:       " << "\xF0\x9F\x94\x93" << "  (U+1F513 🔓)\n";
    cout << "    Lock with key:  " << "\xF0\x9F\x94\x90" << "  (U+1F510 🔐)\n";

    // ========================================================================
    // MEDALS & AWARDS (for achievements)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ MEDALS & AWARDS ═══" << Color::RESET << "\n\n";
    
    cout << "    Gold medal:     " << Color::YELLOW << "\xF0\x9F\xA5\x87" << Color::RESET << "  (U+1F947 🥇)\n";
    cout << "    Silver medal:   " << "\xF0\x9F\xA5\x88" << "  (U+1F948 🥈)\n";
    cout << "    Bronze medal:   " << "\xF0\x9F\xA5\x89" << "  (U+1F949 🥉)\n";
    cout << "    Trophy:         " << Color::YELLOW << "\xF0\x9F\x8F\x86" << Color::RESET << "  (U+1F3C6 🏆)\n";
    cout << "    Sports medal:   " << "\xF0\x9F\x8F\x85" << "  (U+1F3C5 🏅)\n";
    cout << "    Military medal: " << "\xF0\x9F\x8E\x96" << "  (U+1F396 🎖)\n";
    cout << "    Rosette:        " << "\xF0\x9F\x8F\xB5" << "  (U+1F3F5 🏵)\n";
    cout << "    Ribbon:         " << "\xF0\x9F\x8E\x80" << "  (U+1F380 🎀)\n";

    // ========================================================================
    // FOOD & DRINKS (for power-ups/items)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ FOOD & DRINKS ═══" << Color::RESET << "\n\n";
    
    cout << "    Apple:          " << Color::RED << "\xF0\x9F\x8D\x8E" << Color::RESET << "  (U+1F34E 🍎)\n";
    cout << "    Green apple:    " << Color::GREEN << "\xF0\x9F\x8D\x8F" << Color::RESET << "  (U+1F34F 🍏)\n";
    cout << "    Pizza:          " << "\xF0\x9F\x8D\x95" << "  (U+1F355 🍕)\n";
    cout << "    Hamburger:      " << "\xF0\x9F\x8D\x94" << "  (U+1F354 🍔)\n";
    cout << "    Fries:          " << "\xF0\x9F\x8D\x9F" << "  (U+1F35F 🍟)\n";
    cout << "    Hot dog:        " << "\xF0\x9F\x8C\xAD" << "  (U+1F32D 🌭)\n";
    cout << "    Popcorn:        " << "\xF0\x9F\x8D\xBF" << "  (U+1F37F 🍿)\n";
    cout << "    Cookie:         " << "\xF0\x9F\x8D\xAA" << "  (U+1F36A 🍪)\n";
    cout << "    Candy:          " << "\xF0\x9F\x8D\xAC" << "  (U+1F36C 🍬)\n";
    cout << "    Lollipop:       " << "\xF0\x9F\x8D\xAD" << "  (U+1F36D 🍭)\n";
    cout << "    Beer:           " << "\xF0\x9F\x8D\xBA" << "  (U+1F37A 🍺)\n";
    cout << "    Wine:           " << "\xF0\x9F\x8D\xB7" << "  (U+1F377 🍷)\n";
    cout << "    Cocktail:       " << "\xF0\x9F\x8D\xB8" << "  (U+1F378 🍸)\n";
    cout << "    Coffee:         " << "\xE2\x98\x95" << "  (U+2615 ☕)\n";

    // ========================================================================
    // ANIMALS (for mascots/themes)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ ANIMALS ═══" << Color::RESET << "\n\n";
    
    cout << "    Cat:            " << "\xF0\x9F\x90\xB1" << "  (U+1F431 🐱)\n";
    cout << "    Dog:            " << "\xF0\x9F\x90\xB6" << "  (U+1F436 🐶)\n";
    cout << "    Horse:          " << "\xF0\x9F\x90\xB4" << "  (U+1F434 🐴)\n";
    cout << "    Unicorn:        " << "\xF0\x9F\xA6\x84" << "  (U+1F984 🦄)\n";
    cout << "    Dragon:         " << "\xF0\x9F\x90\x89" << "  (U+1F409 🐉)\n";
    cout << "    Snake:          " << "\xF0\x9F\x90\x8D" << "  (U+1F40D 🐍)\n";
    cout << "    Rabbit:         " << "\xF0\x9F\x90\xB0" << "  (U+1F430 🐰)\n";
    cout << "    Fox:            " << "\xF0\x9F\xA6\x8A" << "  (U+1F98A 🦊)\n";
    cout << "    Wolf:           " << "\xF0\x9F\x90\xBA" << "  (U+1F43A 🐺)\n";
    cout << "    Lion:           " << "\xF0\x9F\xA6\x81" << "  (U+1F981 🦁)\n";
    cout << "    Eagle:          " << "\xF0\x9F\xA6\x85" << "  (U+1F985 🦅)\n";
    cout << "    Owl:            " << "\xF0\x9F\xA6\x89" << "  (U+1F989 🦉)\n";

    // ========================================================================
    // WEAPONS & TOOLS (for RPG elements)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ WEAPONS & TOOLS ═══" << Color::RESET << "\n\n";
    
    cout << "    Sword:          " << "\xF0\x9F\x97\xA1" << "  (U+1F5E1 🗡)\n";
    cout << "    Dagger:         " << "\xF0\x9F\x94\xAA" << "  (U+1F52A 🔪)\n";
    cout << "    Shield:         " << "\xF0\x9F\x9B\xA1" << "  (U+1F6E1 🛡)\n";
    cout << "    Bow:            " << "\xF0\x9F\x8F\xB9" << "  (U+1F3F9 🏹)\n";
    cout << "    Bomb:           " << "\xF0\x9F\x92\xA3" << "  (U+1F4A3 💣)\n";
    cout << "    Hammer:         " << "\xF0\x9F\x94\xA8" << "  (U+1F528 🔨)\n";
    cout << "    Wrench:         " << "\xF0\x9F\x94\xA7" << "  (U+1F527 🔧)\n";
    cout << "    Pick:           " << "\xE2\x9B\x8F" << "  (U+26CF ⛏)\n";
    cout << "    Axe:            " << "\xF0\x9F\xAA\x93" << "  (U+1FA93 🪓)\n";
    cout << "    Magic wand:     " << "\xF0\x9F\xAA\x84" << "  (U+1FA84 🪄)\n";
    cout << "    Crystal ball:   " << "\xF0\x9F\x94\xAE" << "  (U+1F52E 🔮)\n";

    // ========================================================================
    // POTIONS & MAGIC (for power-ups)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ POTIONS & MAGIC ═══" << Color::RESET << "\n\n";
    
    cout << "    Potion:         " << "\xF0\x9F\xA7\xAA" << "  (U+1F9EA 🧪)\n";
    cout << "    Alembic:        " << "\xE2\x9A\x97" << "  (U+2697 ⚗)\n";
    cout << "    Sparkles:       " << "\xE2\x9C\xA8" << "  (U+2728 ✨)\n";
    cout << "    Dizzy:          " << "\xF0\x9F\x92\xAB" << "  (U+1F4AB 💫)\n";
    cout << "    Collision:      " << "\xF0\x9F\x92\xA5" << "  (U+1F4A5 💥)\n";
    cout << "    Boom:           " << "\xF0\x9F\x92\xA2" << "  (U+1F4A2 💢)\n";
    cout << "    Sweat drops:    " << "\xF0\x9F\x92\xA6" << "  (U+1F4A6 💦)\n";
    cout << "    Zzz:            " << "\xF0\x9F\x92\xA4" << "  (U+1F4A4 💤)\n";
    cout << "    Thought:        " << "\xF0\x9F\x92\xAD" << "  (U+1F4AD 💭)\n";
    cout << "    Speech:         " << "\xF0\x9F\x92\xAC" << "  (U+1F4AC 💬)\n";

    // ========================================================================
    // TIME & SPEED (for timers/bonuses)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ TIME & SPEED ═══" << Color::RESET << "\n\n";
    
    cout << "    Hourglass:      " << "\xE2\x8C\x9B" << "  (U+231B ⌛)\n";
    cout << "    Hourglass flow: " << "\xE2\x8F\xB3" << "  (U+23F3 ⏳)\n";
    cout << "    Watch:          " << "\xE2\x8C\x9A" << "  (U+231A ⌚)\n";
    cout << "    Alarm clock:    " << "\xE2\x8F\xB0" << "  (U+23F0 ⏰)\n";
    cout << "    Stopwatch:      " << "\xE2\x8F\xB1" << "  (U+23F1 ⏱)\n";
    cout << "    Timer:          " << "\xE2\x8F\xB2" << "  (U+23F2 ⏲)\n";
    cout << "    Fast forward:   " << "\xE2\x8F\xA9" << "  (U+23E9 ⏩)\n";
    cout << "    Rewind:         " << "\xE2\x8F\xAA" << "  (U+23EA ⏪)\n";
    cout << "    Play:           " << "\xE2\x96\xB6" << "  (U+25B6 ▶)\n";
    cout << "    Pause:          " << "\xE2\x8F\xB8" << "  (U+23F8 ⏸)\n";
    cout << "    Stop:           " << "\xE2\x8F\xB9" << "  (U+23F9 ⏹)\n";
    cout << "    Record:         " << "\xE2\x8F\xBA" << "  (U+23FA ⏺)\n";

    // ========================================================================
    // HANDS & GESTURES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ HANDS & GESTURES ═══" << Color::RESET << "\n\n";
    
    cout << "    Point up:       " << "\xE2\x98\x9D" << "  (U+261D ☝)\n";
    cout << "    Point right:    " << "\xF0\x9F\x91\x89" << "  (U+1F449 👉)\n";
    cout << "    Point left:     " << "\xF0\x9F\x91\x88" << "  (U+1F448 👈)\n";
    cout << "    Point down:     " << "\xF0\x9F\x91\x87" << "  (U+1F447 👇)\n";
    cout << "    Victory:        " << "\xE2\x9C\x8C" << "  (U+270C ✌)\n";
    cout << "    Crossed:        " << "\xF0\x9F\xA4\x9E" << "  (U+1F91E 🤞)\n";
    cout << "    OK hand:        " << "\xF0\x9F\x91\x8C" << "  (U+1F44C 👌)\n";
    cout << "    Fist:           " << "\xE2\x9C\x8A" << "  (U+270A ✊)\n";
    cout << "    Raised fist:    " << "\xF0\x9F\x91\x8A" << "  (U+1F44A 👊)\n";
    cout << "    Wave:           " << "\xF0\x9F\x91\x8B" << "  (U+1F44B 👋)\n";
    cout << "    Pray:           " << "\xF0\x9F\x99\x8F" << "  (U+1F64F 🙏)\n";
    cout << "    Muscle:         " << "\xF0\x9F\x92\xAA" << "  (U+1F4AA 💪)\n";

    // ========================================================================
    // PLANETS & SPACE
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ PLANETS & SPACE ═══" << Color::RESET << "\n\n";
    
    cout << "    Sun:            " << Color::YELLOW << "\xE2\x98\x89" << Color::RESET << "  (U+2609 ☉)\n";
    cout << "    Mercury:        " << "\xE2\x98\xBF" << "  (U+263F ☿)\n";
    cout << "    Venus:          " << "\xE2\x99\x80" << "  (U+2640 ♀)\n";
    cout << "    Earth:          " << "\xF0\x9F\x8C\x8D" << "  (U+1F30D 🌍)\n";
    cout << "    Mars:           " << "\xE2\x99\x82" << "  (U+2642 ♂)\n";
    cout << "    Jupiter:        " << "\xE2\x99\x83" << "  (U+2643 ♃)\n";
    cout << "    Saturn:         " << "\xE2\x99\x84" << "  (U+2644 ♄)\n";
    cout << "    Uranus:         " << "\xE2\x99\x85" << "  (U+2645 ♅)\n";
    cout << "    Neptune:        " << "\xE2\x99\x86" << "  (U+2646 ♆)\n";
    cout << "    Pluto:          " << "\xE2\x99\x87" << "  (U+2647 ♇)\n";
    cout << "    Rocket:         " << "\xF0\x9F\x9A\x80" << "  (U+1F680 🚀)\n";
    cout << "    UFO:            " << "\xF0\x9F\x9B\xB8" << "  (U+1F6F8 🛸)\n";
    cout << "    Satellite:      " << "\xF0\x9F\x9B\xB0" << "  (U+1F6F0 🛰)\n";
    cout << "    Milky way:      " << "\xF0\x9F\x8C\x8C" << "  (U+1F30C 🌌)\n";
    cout << "    Shooting star:  " << "\xF0\x9F\x8C\xA0" << "  (U+1F320 🌠)\n";

    // ========================================================================
    // VEHICLES & TRANSPORT
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ VEHICLES & TRANSPORT ═══" << Color::RESET << "\n\n";
    
    cout << "    Car:            " << "\xF0\x9F\x9A\x97" << "  (U+1F697 🚗)\n";
    cout << "    Racing car:     " << "\xF0\x9F\x8F\x8E" << "  (U+1F3CE 🏎)\n";
    cout << "    Motorcycle:     " << "\xF0\x9F\x8F\x8D" << "  (U+1F3CD 🏍)\n";
    cout << "    Bicycle:        " << "\xF0\x9F\x9A\xB2" << "  (U+1F6B2 🚲)\n";
    cout << "    Bus:            " << "\xF0\x9F\x9A\x8C" << "  (U+1F68C 🚌)\n";
    cout << "    Train:          " << "\xF0\x9F\x9A\x82" << "  (U+1F682 🚂)\n";
    cout << "    Airplane:       " << "\xE2\x9C\x88" << "  (U+2708 ✈)\n";
    cout << "    Helicopter:     " << "\xF0\x9F\x9A\x81" << "  (U+1F681 🚁)\n";
    cout << "    Ship:           " << "\xF0\x9F\x9A\xA2" << "  (U+1F6A2 🚢)\n";
    cout << "    Anchor:         " << "\xE2\x9A\x93" << "  (U+2693 ⚓)\n";
    cout << "    Fuel pump:      " << "\xE2\x9B\xBD" << "  (U+26FD ⛽)\n";

    // ========================================================================
    // SPORTS & ACTIVITIES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ SPORTS & ACTIVITIES ═══" << Color::RESET << "\n\n";
    
    cout << "    Soccer:         " << "\xE2\x9A\xBD" << "  (U+26BD ⚽)\n";
    cout << "    Basketball:     " << "\xF0\x9F\x8F\x80" << "  (U+1F3C0 🏀)\n";
    cout << "    Football:       " << "\xF0\x9F\x8F\x88" << "  (U+1F3C8 🏈)\n";
    cout << "    Baseball:       " << "\xE2\x9A\xBE" << "  (U+26BE ⚾)\n";
    cout << "    Tennis:         " << "\xF0\x9F\x8E\xBE" << "  (U+1F3BE 🎾)\n";
    cout << "    Bowling:        " << "\xF0\x9F\x8E\xB3" << "  (U+1F3B3 🎳)\n";
    cout << "    Golf:           " << "\xE2\x9B\xB3" << "  (U+26F3 ⛳)\n";
    cout << "    Pool 8-ball:    " << "\xF0\x9F\x8E\xB1" << "  (U+1F3B1 🎱)\n";
    cout << "    Darts:          " << "\xF0\x9F\x8E\xAF" << "  (U+1F3AF 🎯)\n";
    cout << "    Boxing glove:   " << "\xF0\x9F\xA5\x8A" << "  (U+1F94A 🥊)\n";
    cout << "    Running:        " << "\xF0\x9F\x8F\x83" << "  (U+1F3C3 🏃)\n";
    cout << "    Swimming:       " << "\xF0\x9F\x8F\x8A" << "  (U+1F3CA 🏊)\n";
    cout << "    Skiing:         " << "\xE2\x9B\xB7" << "  (U+26F7 ⛷)\n";

    // ========================================================================
    // BUILDINGS & PLACES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ BUILDINGS & PLACES ═══" << Color::RESET << "\n\n";
    
    cout << "    House:          " << "\xF0\x9F\x8F\xA0" << "  (U+1F3E0 🏠)\n";
    cout << "    Office:         " << "\xF0\x9F\x8F\xA2" << "  (U+1F3E2 🏢)\n";
    cout << "    Hospital:       " << "\xF0\x9F\x8F\xA5" << "  (U+1F3E5 🏥)\n";
    cout << "    Bank:           " << "\xF0\x9F\x8F\xA6" << "  (U+1F3E6 🏦)\n";
    cout << "    Hotel:          " << "\xF0\x9F\x8F\xA8" << "  (U+1F3E8 🏨)\n";
    cout << "    School:         " << "\xF0\x9F\x8F\xAB" << "  (U+1F3EB 🏫)\n";
    cout << "    Factory:        " << "\xF0\x9F\x8F\xAD" << "  (U+1F3ED 🏭)\n";
    cout << "    Castle:         " << "\xF0\x9F\x8F\xB0" << "  (U+1F3F0 🏰)\n";
    cout << "    Church:         " << "\xE2\x9B\xAA" << "  (U+26EA ⛪)\n";
    cout << "    Fountain:       " << "\xE2\x9B\xB2" << "  (U+26F2 ⛲)\n";
    cout << "    Tent:           " << "\xE2\x9B\xBA" << "  (U+26FA ⛺)\n";
    cout << "    Mountain:       " << "\xE2\x9B\xB0" << "  (U+26F0 ⛰)\n";
    cout << "    Volcano:        " << "\xF0\x9F\x8C\x8B" << "  (U+1F30B 🌋)\n";
    cout << "    Desert island:  " << "\xF0\x9F\x8F\x9D" << "  (U+1F3DD 🏝)\n";

    // ========================================================================
    // CLOTHING & ACCESSORIES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CLOTHING & ACCESSORIES ═══" << Color::RESET << "\n\n";
    
    cout << "    Crown:          " << Color::YELLOW << "\xF0\x9F\x91\x91" << Color::RESET << "  (U+1F451 👑)\n";
    cout << "    Top hat:        " << "\xF0\x9F\x8E\xA9" << "  (U+1F3A9 🎩)\n";
    cout << "    Graduation:     " << "\xF0\x9F\x8E\x93" << "  (U+1F393 🎓)\n";
    cout << "    Glasses:        " << "\xF0\x9F\x91\x93" << "  (U+1F453 👓)\n";
    cout << "    Sunglasses:     " << "\xF0\x9F\x95\xB6" << "  (U+1F576 🕶)\n";
    cout << "    Necktie:        " << "\xF0\x9F\x91\x94" << "  (U+1F454 👔)\n";
    cout << "    T-shirt:        " << "\xF0\x9F\x91\x95" << "  (U+1F455 👕)\n";
    cout << "    Dress:          " << "\xF0\x9F\x91\x97" << "  (U+1F457 👗)\n";
    cout << "    Ring:           " << "\xF0\x9F\x92\x8D" << "  (U+1F48D 💍)\n";
    cout << "    Lipstick:       " << "\xF0\x9F\x92\x84" << "  (U+1F484 💄)\n";
    cout << "    High heel:      " << "\xF0\x9F\x91\xA0" << "  (U+1F460 👠)\n";
    cout << "    Boot:           " << "\xF0\x9F\x91\xA2" << "  (U+1F462 👢)\n";

    // ========================================================================
    // MATH & SCIENCE SYMBOLS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ MATH & SCIENCE ═══" << Color::RESET << "\n\n";
    
    cout << "    Plus-minus:     " << "\xC2\xB1" << "  (U+00B1 ±)\n";
    cout << "    Multiply:       " << "\xC3\x97" << "  (U+00D7 ×)\n";
    cout << "    Divide:         " << "\xC3\xB7" << "  (U+00F7 ÷)\n";
    cout << "    Not equal:      " << "\xE2\x89\xA0" << "  (U+2260 ≠)\n";
    cout << "    Less or equal:  " << "\xE2\x89\xA4" << "  (U+2264 ≤)\n";
    cout << "    Greater/equal:  " << "\xE2\x89\xA5" << "  (U+2265 ≥)\n";
    cout << "    Approx:         " << "\xE2\x89\x88" << "  (U+2248 ≈)\n";
    cout << "    Infinity:       " << "\xE2\x88\x9E" << "  (U+221E ∞)\n";
    cout << "    Sum:            " << "\xE2\x88\x91" << "  (U+2211 ∑)\n";
    cout << "    Product:        " << "\xE2\x88\x8F" << "  (U+220F ∏)\n";
    cout << "    Square root:    " << "\xE2\x88\x9A" << "  (U+221A √)\n";
    cout << "    Delta:          " << "\xCE\x94" << "  (U+0394 Δ)\n";
    cout << "    Pi:             " << "\xCF\x80" << "  (U+03C0 π)\n";
    cout << "    Omega:          " << "\xCE\xA9" << "  (U+03A9 Ω)\n";
    cout << "    Micro:          " << "\xC2\xB5" << "  (U+00B5 µ)\n";
    cout << "    Percent:        " << "\xEF\xBC\x85" << "  (U+FF05 ％)\n";
    cout << "    Per mille:      " << "\xE2\x80\xB0" << "  (U+2030 ‰)\n";
    cout << "    Atom:           " << "\xE2\x9A\x9B" << "  (U+269B ⚛)\n";
    cout << "    Biohazard:      " << "\xE2\x98\xA3" << "  (U+2623 ☣)\n";
    cout << "    Radioactive:    " << "\xE2\x98\xA2" << "  (U+2622 ☢)\n";

    // ========================================================================
    // CURRENCY SYMBOLS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CURRENCY SYMBOLS ═══" << Color::RESET << "\n\n";
    
    cout << "    Dollar:         " << Color::GREEN << "$" << Color::RESET << "  (U+0024 $)\n";
    cout << "    Euro:           " << "\xE2\x82\xAC" << "  (U+20AC €)\n";
    cout << "    Pound:          " << "\xC2\xA3" << "  (U+00A3 £)\n";
    cout << "    Yen:            " << "\xC2\xA5" << "  (U+00A5 ¥)\n";
    cout << "    Won:            " << "\xE2\x82\xA9" << "  (U+20A9 ₩)\n";
    cout << "    Rupee:          " << "\xE2\x82\xB9" << "  (U+20B9 ₹)\n";
    cout << "    Bitcoin:        " << Color::YELLOW << "\xE2\x82\xBF" << Color::RESET << "  (U+20BF ₿)\n";
    cout << "    Cent:           " << "\xC2\xA2" << "  (U+00A2 ¢)\n";
    cout << "    Generic:        " << "\xC2\xA4" << "  (U+00A4 ¤)\n";
    cout << "    Peso:           " << "\xE2\x82\xB1" << "  (U+20B1 ₱)\n";
    cout << "    Ruble:          " << "\xE2\x82\xBD" << "  (U+20BD ₽)\n";
    cout << "    Lira:           " << "\xE2\x82\xBA" << "  (U+20BA ₺)\n";

    // ========================================================================
    // RELIGIOUS & SPIRITUAL
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ RELIGIOUS & SPIRITUAL ═══" << Color::RESET << "\n\n";
    
    cout << "    Peace:          " << "\xE2\x98\xAE" << "  (U+262E ☮)\n";
    cout << "    Yin yang:       " << "\xE2\x98\xAF" << "  (U+262F ☯)\n";
    cout << "    Cross:          " << "\xE2\x9C\x9D" << "  (U+271D ✝)\n";
    cout << "    Star of David:  " << "\xE2\x9C\xA1" << "  (U+2721 ✡)\n";
    cout << "    Om:             " << "\xF0\x9F\x95\x89" << "  (U+1F549 🕉)\n";
    cout << "    Wheel dharma:   " << "\xE2\x98\xB8" << "  (U+2638 ☸)\n";
    cout << "    Crescent:       " << "\xE2\x98\xAA" << "  (U+262A ☪)\n";
    cout << "    Ankh:           " << "\xE2\x98\xA5" << "  (U+2625 ☥)\n";
    cout << "    Pray beads:     " << "\xF0\x9F\x93\xBF" << "  (U+1F4FF 📿)\n";

    // ========================================================================
    // COMMUNICATION & TECH
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ COMMUNICATION & TECH ═══" << Color::RESET << "\n\n";
    
    cout << "    Phone:          " << "\xE2\x98\x8E" << "  (U+260E ☎)\n";
    cout << "    Mobile:         " << "\xF0\x9F\x93\xB1" << "  (U+1F4F1 📱)\n";
    cout << "    Computer:       " << "\xF0\x9F\x92\xBB" << "  (U+1F4BB 💻)\n";
    cout << "    Keyboard:       " << "\xE2\x8C\xA8" << "  (U+2328 ⌨)\n";
    cout << "    Mouse:          " << "\xF0\x9F\x96\xB1" << "  (U+1F5B1 🖱)\n";
    cout << "    Printer:        " << "\xF0\x9F\x96\xA8" << "  (U+1F5A8 🖨)\n";
    cout << "    Disk:           " << "\xF0\x9F\x92\xBF" << "  (U+1F4BF 💿)\n";
    cout << "    DVD:            " << "\xF0\x9F\x93\x80" << "  (U+1F4C0 📀)\n";
    cout << "    USB:            " << "\xF0\x9F\x94\x8C" << "  (U+1F50C 🔌)\n";
    cout << "    Battery:        " << "\xF0\x9F\x94\x8B" << "  (U+1F50B 🔋)\n";
    cout << "    Light bulb:     " << Color::YELLOW << "\xF0\x9F\x92\xA1" << Color::RESET << "  (U+1F4A1 💡)\n";
    cout << "    Flashlight:     " << "\xF0\x9F\x94\xA6" << "  (U+1F526 🔦)\n";
    cout << "    Camera:         " << "\xF0\x9F\x93\xB7" << "  (U+1F4F7 📷)\n";
    cout << "    Video camera:   " << "\xF0\x9F\x93\xB9" << "  (U+1F4F9 📹)\n";
    cout << "    TV:             " << "\xF0\x9F\x93\xBA" << "  (U+1F4FA 📺)\n";
    cout << "    Radio:          " << "\xF0\x9F\x93\xBB" << "  (U+1F4FB 📻)\n";
    cout << "    Satellite dish: " << "\xF0\x9F\x93\xA1" << "  (U+1F4E1 📡)\n";
    cout << "    Email:          " << "\xF0\x9F\x93\xA7" << "  (U+1F4E7 📧)\n";
    cout << "    Inbox:          " << "\xF0\x9F\x93\xA5" << "  (U+1F4E5 📥)\n";
    cout << "    Outbox:         " << "\xF0\x9F\x93\xA4" << "  (U+1F4E4 📤)\n";

    // ========================================================================
    // OFFICE & STATIONERY
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ OFFICE & STATIONERY ═══" << Color::RESET << "\n\n";
    
    cout << "    Pencil:         " << "\xE2\x9C\x8F" << "  (U+270F ✏)\n";
    cout << "    Pen:            " << "\xF0\x9F\x96\x8A" << "  (U+1F58A 🖊)\n";
    cout << "    Paintbrush:     " << "\xF0\x9F\x96\x8C" << "  (U+1F58C 🖌)\n";
    cout << "    Crayon:         " << "\xF0\x9F\x96\x8D" << "  (U+1F58D 🖍)\n";
    cout << "    Scissors:       " << "\xE2\x9C\x82" << "  (U+2702 ✂)\n";
    cout << "    Ruler:          " << "\xF0\x9F\x93\x8F" << "  (U+1F4CF 📏)\n";
    cout << "    Paperclip:      " << "\xF0\x9F\x93\x8E" << "  (U+1F4CE 📎)\n";
    cout << "    Pushpin:        " << "\xF0\x9F\x93\x8C" << "  (U+1F4CC 📌)\n";
    cout << "    Clipboard:      " << "\xF0\x9F\x93\x8B" << "  (U+1F4CB 📋)\n";
    cout << "    Notebook:       " << "\xF0\x9F\x93\x93" << "  (U+1F4D3 📓)\n";
    cout << "    Book:           " << "\xF0\x9F\x93\x96" << "  (U+1F4D6 📖)\n";
    cout << "    Books:          " << "\xF0\x9F\x93\x9A" << "  (U+1F4DA 📚)\n";
    cout << "    Scroll:         " << "\xF0\x9F\x93\x9C" << "  (U+1F4DC 📜)\n";
    cout << "    Calendar:       " << "\xF0\x9F\x93\x85" << "  (U+1F4C5 📅)\n";
    cout << "    Folder:         " << "\xF0\x9F\x93\x81" << "  (U+1F4C1 📁)\n";
    cout << "    Trash:          " << "\xF0\x9F\x97\x91" << "  (U+1F5D1 🗑)\n";
    cout << "    Magnifier:      " << "\xF0\x9F\x94\x8D" << "  (U+1F50D 🔍)\n";

    // ========================================================================
    // CELEBRATION & PARTY
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ CELEBRATION & PARTY ═══" << Color::RESET << "\n\n";
    
    cout << "    Party popper:   " << "\xF0\x9F\x8E\x89" << "  (U+1F389 🎉)\n";
    cout << "    Confetti:       " << "\xF0\x9F\x8E\x8A" << "  (U+1F38A 🎊)\n";
    cout << "    Balloon:        " << "\xF0\x9F\x8E\x88" << "  (U+1F388 🎈)\n";
    cout << "    Gift:           " << "\xF0\x9F\x8E\x81" << "  (U+1F381 🎁)\n";
    cout << "    Wrapped gift:   " << "\xF0\x9F\x8E\x80" << "  (U+1F380 🎀)\n";
    cout << "    Cake:           " << "\xF0\x9F\x8E\x82" << "  (U+1F382 🎂)\n";
    cout << "    Candle:         " << "\xF0\x9F\x95\xAF" << "  (U+1F56F 🕯)\n";
    cout << "    Fireworks:      " << "\xF0\x9F\x8E\x86" << "  (U+1F386 🎆)\n";
    cout << "    Sparkler:       " << "\xF0\x9F\x8E\x87" << "  (U+1F387 🎇)\n";
    cout << "    Christmas tree: " << "\xF0\x9F\x8E\x84" << "  (U+1F384 🎄)\n";
    cout << "    Jack-o-lantern: " << "\xF0\x9F\x8E\x83" << "  (U+1F383 🎃)\n";
    cout << "    Santa:          " << "\xF0\x9F\x8E\x85" << "  (U+1F385 🎅)\n";

    // ========================================================================
    // FLAGS & BANNERS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ FLAGS & BANNERS ═══" << Color::RESET << "\n\n";
    
    cout << "    White flag:     " << "\xF0\x9F\x8F\xB3" << "  (U+1F3F3 🏳)\n";
    cout << "    Black flag:     " << "\xF0\x9F\x8F\xB4" << "  (U+1F3F4 🏴)\n";
    cout << "    Checkered:      " << "\xF0\x9F\x8F\x81" << "  (U+1F3C1 🏁)\n";
    cout << "    Triangular:     " << "\xF0\x9F\x9A\xA9" << "  (U+1F6A9 🚩)\n";
    cout << "    Crossed flags:  " << "\xF0\x9F\x8E\x8C" << "  (U+1F38C 🎌)\n";
    cout << "    Pirate:         " << "\xF0\x9F\x8F\xB4\xE2\x80\x8D\xE2\x98\xA0\xEF\xB8\x8F" << "  (🏴‍☠️)\n";

    // ========================================================================
    // DANGER & WARNING
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ DANGER & WARNING ═══" << Color::RESET << "\n\n";
    
    cout << "    Warning:        " << Color::YELLOW << "\xE2\x9A\xA0" << Color::RESET << "  (U+26A0 ⚠)\n";
    cout << "    No entry:       " << Color::RED << "\xE2\x9B\x94" << Color::RESET << "  (U+26D4 ⛔)\n";
    cout << "    Prohibited:     " << Color::RED << "\xF0\x9F\x9A\xAB" << Color::RESET << "  (U+1F6AB 🚫)\n";
    cout << "    Stop sign:      " << Color::RED << "\xF0\x9F\x9B\x91" << Color::RESET << "  (U+1F6D1 🛑)\n";
    cout << "    Skull bones:    " << "\xE2\x98\xA0" << "  (U+2620 ☠)\n";
    cout << "    High voltage:   " << Color::YELLOW << "\xE2\x9A\xA1" << Color::RESET << "  (U+26A1 ⚡)\n";
    cout << "    Biohazard:      " << Color::YELLOW << "\xE2\x98\xA3" << Color::RESET << "  (U+2623 ☣)\n";
    cout << "    Radioactive:    " << Color::YELLOW << "\xE2\x98\xA2" << Color::RESET << "  (U+2622 ☢)\n";
    cout << "    Construction:   " << "\xF0\x9F\x9A\xA7" << "  (U+1F6A7 🚧)\n";
    cout << "    Police light:   " << "\xF0\x9F\x9A\xA8" << "  (U+1F6A8 🚨)\n";

    // ========================================================================
    // ROULETTE NUMBERS (for roulette game)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ ROULETTE COLORS ═══" << Color::RESET << "\n\n";
    
    cout << "  Red numbers (1,3,5,7,9,12,14,16,18,19,21,23,25,27,30,32,34,36):\n";
    cout << "    " << Color::RED << "\xE2\x97\x8F" << Color::RESET << " Red circle     (U+25CF ●)\n";
    cout << "    " << Color::RED << "\xE2\x96\xA0" << Color::RESET << " Red square     (U+25A0 ■)\n";
    cout << "    " << Color::RED << "\xE2\x99\xA6" << Color::RESET << " Red diamond    (U+2666 ♦)\n";
    
    cout << "\n  Black numbers (2,4,6,8,10,11,13,15,17,20,22,24,26,28,29,31,33,35):\n";
    cout << "    " << "\xE2\x97\x8F" << " Black circle   (U+25CF ●)\n";
    cout << "    " << "\xE2\x96\xA0" << " Black square   (U+25A0 ■)\n";
    cout << "    " << "\xE2\x99\xA0" << " Black spade    (U+2660 ♠)\n";
    
    cout << "\n  Green (0, 00):\n";
    cout << "    " << Color::GREEN << "\xE2\x97\x8F" << Color::RESET << " Green circle   (U+25CF ●)\n";
    cout << "    " << Color::GREEN << "0" << Color::RESET << " Zero\n";

    // ========================================================================
    // ROMAN NUMERALS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ ROMAN NUMERALS ═══" << Color::RESET << "\n\n";
    
    cout << "    I:    " << "\xE2\x85\xA0" << "  II:   " << "\xE2\x85\xA1" << "  III:  " << "\xE2\x85\xA2" << "  IV:   " << "\xE2\x85\xA3" << "  V:    " << "\xE2\x85\xA4" << "\n";
    cout << "    VI:   " << "\xE2\x85\xA5" << "  VII:  " << "\xE2\x85\xA6" << "  VIII: " << "\xE2\x85\xA7" << "  IX:   " << "\xE2\x85\xA8" << "  X:    " << "\xE2\x85\xA9" << "\n";
    cout << "    XI:   " << "\xE2\x85\xAA" << "  XII:  " << "\xE2\x85\xAB" << "  L:    " << "\xE2\x85\xAC" << "  C:    " << "\xE2\x85\xAD" << "  D:    " << "\xE2\x85\xAE" << "  M:    " << "\xE2\x85\xAF" << "\n";

    // ========================================================================
    // BRAILLE PATTERNS (for loading animations)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ BRAILLE PATTERNS ═══" << Color::RESET << "\n\n";
    
    cout << "  Loading spinner frames:\n";
    cout << "    " << "\xE2\xA0\x8B" << " " << "\xE2\xA0\x99" << " " << "\xE2\xA0\xB9" << " " << "\xE2\xA0\xB8" << " " << "\xE2\xA0\xBC" << " " << "\xE2\xA0\xB4" << " " << "\xE2\xA0\xA6" << " " << "\xE2\xA0\xA7" << " " << "\xE2\xA0\x87" << " " << "\xE2\xA0\x8F" << "\n";
    cout << "  Dots pattern:\n";
    cout << "    " << "\xE2\xA0\x80" << " " << "\xE2\xA0\x81" << " " << "\xE2\xA0\x82" << " " << "\xE2\xA0\x83" << " " << "\xE2\xA0\x84" << " " << "\xE2\xA0\x85" << " " << "\xE2\xA0\x86" << " " << "\xE2\xA0\x87" << "\n";
    cout << "  Full block:   " << "\xE2\xA3\xBF" << "  (U+28FF ⣿)\n";

    // ========================================================================
    // GEOMETRIC SHAPES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ GEOMETRIC SHAPES ═══" << Color::RESET << "\n\n";
    
    cout << "  Circles:\n";
    cout << "    Large:      " << "\xE2\x97\x8B" << " " << "\xE2\x97\x8F" << " " << "\xE2\x97\x8C" << " " << "\xE2\x97\x8D" << "  (○●◌◍)\n";
    cout << "    Dotted:     " << "\xE2\x97\x8C" << " " << "\xE2\x97\x8D" << " " << "\xE2\x97\x8E" << " " << "\xE2\x97\x89" << "  (◌◍◎◉)\n";
    cout << "    With dot:   " << "\xE2\x8A\x99" << " " << "\xE2\x8A\x9A" << " " << "\xE2\x8A\x9B" << "  (⊙⊚⊛)\n";
    
    cout << "\n  Squares:\n";
    cout << "    Filled:     " << "\xE2\x96\xA0" << " " << "\xE2\x96\xA1" << " " << "\xE2\x96\xA2" << " " << "\xE2\x96\xA3" << "  (■□▢▣)\n";
    cout << "    Rounded:    " << "\xE2\x96\xA2" << " " << "\xE2\x96\xA3" << " " << "\xE2\x96\xA4" << " " << "\xE2\x96\xA5" << "  (▢▣▤▥)\n";
    cout << "    Patterns:   " << "\xE2\x96\xA4" << " " << "\xE2\x96\xA5" << " " << "\xE2\x96\xA6" << " " << "\xE2\x96\xA7" << " " << "\xE2\x96\xA8" << " " << "\xE2\x96\xA9" << "  (▤▥▦▧▨▩)\n";
    
    cout << "\n  Triangles:\n";
    cout << "    Solid:      " << "\xE2\x96\xB2" << " " << "\xE2\x96\xBC" << " " << "\xE2\x97\x80" << " " << "\xE2\x96\xB6" << "  (▲▼◀▶)\n";
    cout << "    Outline:    " << "\xE2\x96\xB3" << " " << "\xE2\x96\xBD" << " " << "\xE2\x97\x81" << " " << "\xE2\x96\xB7" << "  (△▽◁▷)\n";
    cout << "    Small:      " << "\xE2\x97\xA2" << " " << "\xE2\x97\xA3" << " " << "\xE2\x97\xA4" << " " << "\xE2\x97\xA5" << "  (◢◣◤◥)\n";
    
    cout << "\n  Diamonds:\n";
    cout << "    Solid:      " << "\xE2\x97\x86" << " " << "\xE2\x97\x87" << "  (◆◇)\n";
    cout << "    With dot:   " << "\xE2\x97\x88" << "  (◈)\n";
    
    cout << "\n  Pentagons/Hexagons:\n";
    cout << "    Pentagon:   " << "\xE2\xAC\xA0" << " " << "\xE2\xAC\xA1" << "  (⬠⬡)\n";
    cout << "    Hexagon:    " << "\xE2\xAC\xA2" << " " << "\xE2\xAC\xA3" << "  (⬢⬣)\n";

    // ========================================================================
    // LINE DRAWING EXTENDED
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ LINE DRAWING EXTENDED ═══" << Color::RESET << "\n\n";
    
    cout << "  Heavy lines:\n";
    cout << "    Horizontal: " << "\xE2\x94\x81" << "  Vertical:   " << "\xE2\x94\x83" << "  (━┃)\n";
    cout << "    Corners:    " << "\xE2\x94\x8F" << " " << "\xE2\x94\x93" << " " << "\xE2\x94\x97" << " " << "\xE2\x94\x9B" << "  (┏┓┗┛)\n";
    cout << "    T-joins:    " << "\xE2\x94\xB3" << " " << "\xE2\x94\xBB" << " " << "\xE2\x94\xA3" << " " << "\xE2\x94\xAB" << "  (┳┻┣┫)\n";
    cout << "    Cross:      " << "\xE2\x95\x8B" << "  (╋)\n";
    
    cout << "\n  Dashed lines:\n";
    cout << "    Light dash: " << "\xE2\x94\x84" << " " << "\xE2\x94\x85" << " " << "\xE2\x94\x86" << " " << "\xE2\x94\x87" << "  (┄┅┆┇)\n";
    cout << "    Heavy dash: " << "\xE2\x94\x88" << " " << "\xE2\x94\x89" << " " << "\xE2\x94\x8A" << " " << "\xE2\x94\x8B" << "  (┈┉┊┋)\n";
    
    cout << "\n  Rounded corners:\n";
    cout << "    " << "\xE2\x95\xAD" << "\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80" << "\xE2\x95\xAE" << "\n";
    cout << "    " << "\xE2\x94\x82" << "   " << "\xE2\x94\x82" << "\n";
    cout << "    " << "\xE2\x95\xB0" << "\xE2\x94\x80\xE2\x94\x80\xE2\x94\x80" << "\xE2\x95\xAF" << "  (╭╮╰╯)\n";

    // ========================================================================
    // SPINNERS & LOADING
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ SPINNERS & LOADING ═══" << Color::RESET << "\n\n";
    
    cout << "  Clock faces (for time-based spinners):\n";
    cout << "    " << "\xF0\x9F\x95\x90" << " " << "\xF0\x9F\x95\x91" << " " << "\xF0\x9F\x95\x92" << " " << "\xF0\x9F\x95\x93" << " " << "\xF0\x9F\x95\x94" << " " << "\xF0\x9F\x95\x95" << " " << "\xF0\x9F\x95\x96" << " " << "\xF0\x9F\x95\x97" << " " << "\xF0\x9F\x95\x98" << " " << "\xF0\x9F\x95\x99" << " " << "\xF0\x9F\x95\x9A" << " " << "\xF0\x9F\x95\x9B" << "\n";
    cout << "    (🕐🕑🕒🕓🕔🕕🕖🕗🕘🕙🕚🕛)\n";
    
    cout << "\n  Moon phases:\n";
    cout << "    " << "\xF0\x9F\x8C\x91" << " " << "\xF0\x9F\x8C\x92" << " " << "\xF0\x9F\x8C\x93" << " " << "\xF0\x9F\x8C\x94" << " " << "\xF0\x9F\x8C\x95" << " " << "\xF0\x9F\x8C\x96" << " " << "\xF0\x9F\x8C\x97" << " " << "\xF0\x9F\x8C\x98" << "\n";
    cout << "    (🌑🌒🌓🌔🌕🌖🌗🌘)\n";
    
    cout << "\n  Circle quarters:\n";
    cout << "    " << "\xE2\x97\x94" << " " << "\xE2\x97\x95" << " " << "\xE2\x97\x96" << " " << "\xE2\x97\x97" << "  (◔◕◖◗)\n";
    
    cout << "\n  Arc spinner:\n";
    cout << "    " << "\xE2\x97\x9C" << " " << "\xE2\x97\x9D" << " " << "\xE2\x97\x9E" << " " << "\xE2\x97\x9F" << "  (◜◝◞◟)\n";

    // ========================================================================
    // EMOTICONS (Text-based)
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ EMOTICONS (Text-based) ═══" << Color::RESET << "\n\n";
    
    cout << "    White smile:    " << "\xE2\x98\xBA" << "  (U+263A ☺)\n";
    cout << "    Black smile:    " << "\xE2\x98\xBB" << "  (U+263B ☻)\n";
    cout << "    Frown:          " << "\xE2\x98\xB9" << "  (U+2639 ☹)\n";
    cout << "    Skull:          " << "\xE2\x98\xA0" << "  (U+2620 ☠)\n";
    cout << "    Hot springs:    " << "\xE2\x99\xA8" << "  (U+2668 ♨)\n";

    // ========================================================================
    // MAHJONG TILES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ MAHJONG TILES ═══" << Color::RESET << "\n\n";
    
    cout << "    Red dragon:     " << "\xF0\x9F\x80\x84" << "  (U+1F004 🀄)\n";
    cout << "    Tile back:      " << "\xF0\x9F\x80\xAB" << "  (U+1F02B 🀫)\n";
    cout << "    East wind:      " << "\xF0\x9F\x80\x80" << "  (U+1F000 🀀)\n";
    cout << "    South wind:     " << "\xF0\x9F\x80\x81" << "  (U+1F001 🀁)\n";
    cout << "    West wind:      " << "\xF0\x9F\x80\x82" << "  (U+1F002 🀂)\n";
    cout << "    North wind:     " << "\xF0\x9F\x80\x83" << "  (U+1F003 🀃)\n";

    // ========================================================================
    // DOMINO TILES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ DOMINO TILES ═══" << Color::RESET << "\n\n";
    
    cout << "  Horizontal:\n";
    cout << "    " << "\xF0\x9F\x81\xA3" << " " << "\xF0\x9F\x81\xA4" << " " << "\xF0\x9F\x81\xA5" << " " << "\xF0\x9F\x81\xA6" << " " << "\xF0\x9F\x81\xA7" << " " << "\xF0\x9F\x81\xA8" << " " << "\xF0\x9F\x81\xA9" << "\n";
    cout << "    (🁣🁤🁥🁦🁧🁨🁩 = 0-0 to 0-6)\n";
    
    cout << "  Vertical:\n";
    cout << "    " << "\xF0\x9F\x80\xB1" << " " << "\xF0\x9F\x80\xB2" << " " << "\xF0\x9F\x80\xB3" << " " << "\xF0\x9F\x80\xB4" << " " << "\xF0\x9F\x80\xB5" << " " << "\xF0\x9F\x80\xB6" << " " << "\xF0\x9F\x80\xB7" << "\n";
    cout << "    (🀱🀲🀳🀴🀵🀶🀷 = 0-0 to 0-6)\n";

    // ========================================================================
    // ELECTRICAL & ENGINEERING
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ ELECTRICAL & ENGINEERING ═══" << Color::RESET << "\n\n";
    
    cout << "    Ground:         " << "\xE2\x8F\x9A" << "  (U+23DA ⏚)\n";
    cout << "    Fuse:           " << "\xE2\x8F\x9B" << "  (U+23DB ⏛)\n";
    cout << "    Power:          " << "\xE2\x8F\xBB" << "  (U+23FB ⏻)\n";
    cout << "    Power on:       " << "\xE2\x8F\xBC" << "  (U+23FC ⏼)\n";
    cout << "    Power off:      " << "\xE2\xAD\x98" << "  (U+2B58 ⭘)\n";
    cout << "    Sleep:          " << "\xE2\x8F\xBE" << "  (U+23FE ⏾)\n";
    cout << "    Eject:          " << "\xE2\x8F\x8F" << "  (U+23CF ⏏)\n";

    // ========================================================================
    // RECYCLING SYMBOLS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ RECYCLING SYMBOLS ═══" << Color::RESET << "\n\n";
    
    cout << "    Recycle:        " << Color::GREEN << "\xE2\x99\xBB" << Color::RESET << "  (U+267B ♻)\n";
    cout << "    Recycled paper: " << "\xE2\x99\xBC" << "  (U+267C ♼)\n";
    cout << "    Partial recycle:" << "\xE2\x99\xBD" << "  (U+267D ♽)\n";

    // ========================================================================
    // GENDER & RELATIONSHIP
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ GENDER & RELATIONSHIP ═══" << Color::RESET << "\n\n";
    
    cout << "    Male:           " << Color::BLUE << "\xE2\x99\x82" << Color::RESET << "  (U+2642 ♂)\n";
    cout << "    Female:         " << Color::MAGENTA << "\xE2\x99\x80" << Color::RESET << "  (U+2640 ♀)\n";
    cout << "    Transgender:    " << "\xE2\x9A\xA7" << "  (U+26A7 ⚧)\n";
    cout << "    Couple:         " << "\xF0\x9F\x91\xAB" << "  (U+1F46B 👫)\n";
    cout << "    Family:         " << "\xF0\x9F\x91\xAA" << "  (U+1F46A 👪)\n";
    cout << "    Kiss:           " << "\xF0\x9F\x92\x8B" << "  (U+1F48B 💋)\n";
    cout << "    Love letter:    " << "\xF0\x9F\x92\x8C" << "  (U+1F48C 💌)\n";

    // ========================================================================
    // HANDS EXTENDED
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ HANDS EXTENDED ═══" << Color::RESET << "\n\n";
    
    cout << "    Writing:        " << "\xE2\x9C\x8D" << "  (U+270D ✍)\n";
    cout << "    Nail polish:    " << "\xF0\x9F\x92\x85" << "  (U+1F485 💅)\n";
    cout << "    Selfie:         " << "\xF0\x9F\xA4\xB3" << "  (U+1F933 🤳)\n";
    cout << "    Call me:        " << "\xF0\x9F\xA4\x99" << "  (U+1F919 🤙)\n";
    cout << "    Vulcan:         " << "\xF0\x9F\x96\x96" << "  (U+1F596 🖖)\n";
    cout << "    Metal:          " << "\xF0\x9F\xA4\x98" << "  (U+1F918 🤘)\n";
    cout << "    Love you:       " << "\xF0\x9F\xA4\x9F" << "  (U+1F91F 🤟)\n";
    cout << "    Pinching:       " << "\xF0\x9F\xA4\x8F" << "  (U+1F90F 🤏)\n";
    cout << "    Palms up:       " << "\xF0\x9F\xA4\xB2" << "  (U+1F932 🤲)\n";
    cout << "    Handshake:      " << "\xF0\x9F\xA4\x9D" << "  (U+1F91D 🤝)\n";

    // ========================================================================
    // FANTASY & MYTHOLOGY
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ FANTASY & MYTHOLOGY ═══" << Color::RESET << "\n\n";
    
    cout << "    Dragon:         " << "\xF0\x9F\x90\x89" << "  (U+1F409 🐉)\n";
    cout << "    Dragon face:    " << "\xF0\x9F\x90\xB2" << "  (U+1F432 🐲)\n";
    cout << "    Unicorn:        " << "\xF0\x9F\xA6\x84" << "  (U+1F984 🦄)\n";
    cout << "    Phoenix:        " << "\xF0\x9F\xA6\x85" << "  (U+1F985 🦅)\n";
    cout << "    Mermaid:        " << "\xF0\x9F\xA7\x9C" << "  (U+1F9DC 🧜)\n";
    cout << "    Fairy:          " << "\xF0\x9F\xA7\x9A" << "  (U+1F9DA 🧚)\n";
    cout << "    Vampire:        " << "\xF0\x9F\xA7\x9B" << "  (U+1F9DB 🧛)\n";
    cout << "    Zombie:         " << "\xF0\x9F\xA7\x9F" << "  (U+1F9DF 🧟)\n";
    cout << "    Genie:          " << "\xF0\x9F\xA7\x9E" << "  (U+1F9DE 🧞)\n";
    cout << "    Elf:            " << "\xF0\x9F\xA7\x9D" << "  (U+1F9DD 🧝)\n";
    cout << "    Wizard:         " << "\xF0\x9F\xA7\x99" << "  (U+1F9D9 🧙)\n";
    cout << "    Ghost:          " << "\xF0\x9F\x91\xBB" << "  (U+1F47B 👻)\n";
    cout << "    Alien:          " << "\xF0\x9F\x91\xBD" << "  (U+1F47D 👽)\n";
    cout << "    Robot:          " << "\xF0\x9F\xA4\x96" << "  (U+1F916 🤖)\n";
    cout << "    Ogre:           " << "\xF0\x9F\x91\xB9" << "  (U+1F479 👹)\n";
    cout << "    Goblin:         " << "\xF0\x9F\x91\xBA" << "  (U+1F47A 👺)\n";

    // ========================================================================
    // INSECTS & BUGS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ INSECTS & BUGS ═══" << Color::RESET << "\n\n";
    
    cout << "    Ant:            " << "\xF0\x9F\x90\x9C" << "  (U+1F41C 🐜)\n";
    cout << "    Bee:            " << "\xF0\x9F\x90\x9D" << "  (U+1F41D 🐝)\n";
    cout << "    Beetle:         " << "\xF0\x9F\xAA\xB2" << "  (U+1FAB2 🪲)\n";
    cout << "    Butterfly:      " << "\xF0\x9F\xA6\x8B" << "  (U+1F98B 🦋)\n";
    cout << "    Caterpillar:    " << "\xF0\x9F\x90\x9B" << "  (U+1F41B 🐛)\n";
    cout << "    Cricket:        " << "\xF0\x9F\xA6\x97" << "  (U+1F997 🦗)\n";
    cout << "    Ladybug:        " << "\xF0\x9F\x90\x9E" << "  (U+1F41E 🐞)\n";
    cout << "    Mosquito:       " << "\xF0\x9F\xA6\x9F" << "  (U+1F99F 🦟)\n";
    cout << "    Spider:         " << "\xF0\x9F\x95\xB7" << "  (U+1F577 🕷)\n";
    cout << "    Spider web:     " << "\xF0\x9F\x95\xB8" << "  (U+1F578 🕸)\n";
    cout << "    Scorpion:       " << "\xF0\x9F\xA6\x82" << "  (U+1F982 🦂)\n";
    cout << "    Snail:          " << "\xF0\x9F\x90\x8C" << "  (U+1F40C 🐌)\n";

    // ========================================================================
    // SEA CREATURES
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ SEA CREATURES ═══" << Color::RESET << "\n\n";
    
    cout << "    Fish:           " << "\xF0\x9F\x90\x9F" << "  (U+1F41F 🐟)\n";
    cout << "    Tropical fish:  " << "\xF0\x9F\x90\xA0" << "  (U+1F420 🐠)\n";
    cout << "    Blowfish:       " << "\xF0\x9F\x90\xA1" << "  (U+1F421 🐡)\n";
    cout << "    Shark:          " << "\xF0\x9F\xA6\x88" << "  (U+1F988 🦈)\n";
    cout << "    Whale:          " << "\xF0\x9F\x90\x8B" << "  (U+1F40B 🐋)\n";
    cout << "    Dolphin:        " << "\xF0\x9F\x90\xAC" << "  (U+1F42C 🐬)\n";
    cout << "    Octopus:        " << "\xF0\x9F\x90\x99" << "  (U+1F419 🐙)\n";
    cout << "    Squid:          " << "\xF0\x9F\xA6\x91" << "  (U+1F991 🦑)\n";
    cout << "    Crab:           " << "\xF0\x9F\xA6\x80" << "  (U+1F980 🦀)\n";
    cout << "    Lobster:        " << "\xF0\x9F\xA6\x9E" << "  (U+1F99E 🦞)\n";
    cout << "    Shrimp:         " << "\xF0\x9F\xA6\x90" << "  (U+1F990 🦐)\n";
    cout << "    Jellyfish:      " << "\xF0\x9F\xAA\xBC" << "  (U+1FABC 🪼)\n";
    cout << "    Shell:          " << "\xF0\x9F\x90\x9A" << "  (U+1F41A 🐚)\n";
    cout << "    Coral:          " << "\xF0\x9F\xAA\xB8" << "  (U+1FAB8 🪸)\n";

    // ========================================================================
    // PLANTS & FLOWERS
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::YELLOW << "═══ PLANTS & FLOWERS ═══" << Color::RESET << "\n\n";
    
    cout << "    Seedling:       " << Color::GREEN << "\xF0\x9F\x8C\xB1" << Color::RESET << "  (U+1F331 🌱)\n";
    cout << "    Tree:           " << Color::GREEN << "\xF0\x9F\x8C\xB2" << Color::RESET << "  (U+1F332 🌲)\n";
    cout << "    Palm tree:      " << "\xF0\x9F\x8C\xB4" << "  (U+1F334 🌴)\n";
    cout << "    Cactus:         " << Color::GREEN << "\xF0\x9F\x8C\xB5" << Color::RESET << "  (U+1F335 🌵)\n";
    cout << "    Herb:           " << Color::GREEN << "\xF0\x9F\x8C\xBF" << Color::RESET << "  (U+1F33F 🌿)\n";
    cout << "    Shamrock:       " << Color::GREEN << "\xE2\x98\x98" << Color::RESET << "  (U+2618 ☘)\n";
    cout << "    Four leaf:      " << Color::GREEN << "\xF0\x9F\x8D\x80" << Color::RESET << "  (U+1F340 🍀)\n";
    cout << "    Maple leaf:     " << Color::RED << "\xF0\x9F\x8D\x81" << Color::RESET << "  (U+1F341 🍁)\n";
    cout << "    Fallen leaf:    " << "\xF0\x9F\x8D\x82" << "  (U+1F342 🍂)\n";
    cout << "    Rose:           " << Color::RED << "\xF0\x9F\x8C\xB9" << Color::RESET << "  (U+1F339 🌹)\n";
    cout << "    Tulip:          " << "\xF0\x9F\x8C\xB7" << "  (U+1F337 🌷)\n";
    cout << "    Sunflower:      " << Color::YELLOW << "\xF0\x9F\x8C\xBB" << Color::RESET << "  (U+1F33B 🌻)\n";
    cout << "    Hibiscus:       " << "\xF0\x9F\x8C\xBA" << "  (U+1F33A 🌺)\n";
    cout << "    Cherry blossom: " << "\xF0\x9F\x8C\xB8" << "  (U+1F338 🌸)\n";
    cout << "    Bouquet:        " << "\xF0\x9F\x92\x90" << "  (U+1F490 💐)\n";
    cout << "    Mushroom:       " << "\xF0\x9F\x8D\x84" << "  (U+1F344 🍄)\n";

    // ========================================================================
    // SUMMARY
    // ========================================================================
    cout << "\n" << Color::BOLD << Color::CYAN;
    cout << "╔══════════════════════════════════════════════════════════════╗\n";
    cout << "║                        TEST COMPLETE                         ║\n";
    cout << "╚══════════════════════════════════════════════════════════════╝\n";
    cout << Color::RESET << "\n";
    
    cout << "If all symbols displayed correctly, your terminal is compatible!\n";
    cout << "If some symbols appear as " << Color::RED << "□" << Color::RESET << " or " << Color::RED << "?" << Color::RESET << ", those may not work.\n\n";
    
    cout << Color::DIM << "Terminal requirements:\n";
    cout << "  - UTF-8 encoding enabled\n";
    cout << "  - Font with Unicode support (e.g., Noto, DejaVu, Fira Code)\n";
    cout << "  - For emoji: Font with emoji support or emoji fallback\n";
    cout << Color::RESET << "\n";

    return 0;
}
