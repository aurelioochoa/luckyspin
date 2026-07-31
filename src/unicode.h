/*
 * unicode.h - Unicode character constants for Lucky Spin Casino
 * 
 * This file contains all Unicode characters used in the game, defined using
 * their hex escape codes for better portability and code clarity.
 * 
 * Format: \xNN for single-byte, or raw UTF-8 byte sequences
 * Reference: Unicode/00-unicode-cheat-sheet.md
 */

#ifndef UNICODE_H
#define UNICODE_H

#include <string>

namespace Unicode {
    // ========================================================================
    // BOX DRAWING CHARACTERS (U+2500 block)
    // ========================================================================
    // UTF-8: E2 94 XX
    const std::string H_LINE    = "\xE2\x94\x80";  // ─ U+2500 BOX DRAWINGS LIGHT HORIZONTAL
    const std::string V_LINE    = "\xE2\x94\x82";  // │ U+2502 BOX DRAWINGS LIGHT VERTICAL
    const std::string TL_CORNER = "\xE2\x94\x8C";  // ┌ U+250C BOX DRAWINGS LIGHT DOWN AND RIGHT
    const std::string TR_CORNER = "\xE2\x94\x90";  // ┐ U+2510 BOX DRAWINGS LIGHT DOWN AND LEFT
    const std::string BL_CORNER = "\xE2\x94\x94";  // └ U+2514 BOX DRAWINGS LIGHT UP AND RIGHT
    const std::string BR_CORNER = "\xE2\x94\x98";  // ┘ U+2518 BOX DRAWINGS LIGHT UP AND LEFT
    const std::string T_JOIN    = "\xE2\x94\xAC";  // ┬ U+252C BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
    const std::string B_JOIN    = "\xE2\x94\xB4";  // ┴ U+2534 BOX DRAWINGS LIGHT UP AND HORIZONTAL
    const std::string L_JOIN    = "\xE2\x94\x9C";  // ├ U+251C BOX DRAWINGS LIGHT VERTICAL AND RIGHT
    const std::string R_JOIN    = "\xE2\x94\xA4";  // ┤ U+2524 BOX DRAWINGS LIGHT VERTICAL AND LEFT
    const std::string CROSS     = "\xE2\x94\xBC";  // ┼ U+253C BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL

    // ========================================================================
    // CARD SUIT SYMBOLS (U+2660 block)
    // ========================================================================
    // UTF-8: E2 99 XX
    const std::string SPADE   = "\xE2\x99\xA0";  // ♠ U+2660 BLACK SPADE SUIT
    const std::string HEART   = "\xE2\x99\xA5";  // ♥ U+2665 BLACK HEART SUIT
    const std::string DIAMOND = "\xE2\x99\xA6";  // ♦ U+2666 BLACK DIAMOND SUIT
    const std::string CLUB    = "\xE2\x99\xA3";  // ♣ U+2663 BLACK CLUB SUIT

    // ========================================================================
    // STAR SYMBOLS (U+2605 block)
    // ========================================================================
    // UTF-8: E2 98 XX
    const std::string BLACK_STAR = "\xE2\x98\x85";  // ★ U+2605 BLACK STAR
    const std::string WHITE_STAR = "\xE2\x98\x86";  // ☆ U+2606 WHITE STAR

    // ========================================================================
    // ARROWS AND TRIANGLES (U+25XX block)
    // ========================================================================
    const std::string ARROW_RIGHT = "\xE2\x96\xB6";  // ▶ U+25B6 BLACK RIGHT-POINTING TRIANGLE
    const std::string ARROW_LEFT  = "\xE2\x97\x80";  // ◀ U+25C0 BLACK LEFT-POINTING TRIANGLE
    const std::string ARROW_UP    = "\xE2\x96\xB2";  // ▲ U+25B2 BLACK UP-POINTING TRIANGLE
    const std::string ARROW_DOWN  = "\xE2\x96\xBC";  // ▼ U+25BC BLACK DOWN-POINTING TRIANGLE

    // ========================================================================
    // CHECK MARKS AND CROSSES
    // ========================================================================
    const std::string CHECK    = "\xE2\x9C\x93";  // ✓ U+2713 CHECK MARK
    const std::string CROSS_X  = "\xE2\x9C\x97";  // ✗ U+2717 BALLOT X
    const std::string HEAVY_CHECK = "\xE2\x9C\x94";  // ✔ U+2714 HEAVY CHECK MARK

    // ========================================================================
    // MISCELLANEOUS SYMBOLS
    // ========================================================================
    const std::string BULLET   = "\xE2\x80\xA2";  // • U+2022 BULLET
    const std::string ELLIPSIS = "\xE2\x80\xA6";  // … U+2026 HORIZONTAL ELLIPSIS

    // ========================================================================
    // SLOT MACHINE SYMBOLS (Emoji - 4-byte UTF-8)
    // ========================================================================
    // These are emoji characters that may not render in all terminals
    // Fallback ASCII versions are provided as comments
    const std::string CHERRY   = "\xF0\x9F\x8D\x92";  // 🍒 U+1F352 CHERRIES
    const std::string ORANGE   = "\xF0\x9F\x8D\x8A";  // 🍊 U+1F34A TANGERINE
    const std::string LEMON    = "\xF0\x9F\x8D\x8B";  // 🍋 U+1F34B LEMON
    const std::string GRAPE    = "\xF0\x9F\x8D\x87";  // 🍇 U+1F347 GRAPES
    const std::string BELL     = "\xF0\x9F\x94\x94";  // 🔔 U+1F514 BELL
    const std::string SEVEN    = "7\xEF\xB8\x8F\xE2\x83\xA3";  // 7️⃣ KEYCAP DIGIT SEVEN
    const std::string GEM      = "\xF0\x9F\x92\x8E";  // 💎 U+1F48E GEM STONE
    const std::string STAR_EMOJI = "\xE2\xAD\x90";    // ⭐ U+2B50 WHITE MEDIUM STAR

    // ========================================================================
    // UI ICONS (Emoji)
    // ========================================================================
    const std::string MONEY_BAG  = "\xF0\x9F\x92\xB0";  // 💰 U+1F4B0 MONEY BAG
    const std::string TROPHY     = "\xF0\x9F\x8F\x86";  // 🏆 U+1F3C6 TROPHY
    const std::string DICE       = "\xF0\x9F\x8E\xB2";  // 🎲 U+1F3B2 GAME DIE
    const std::string CHART      = "\xF0\x9F\x93\x88";  // 📈 U+1F4C8 CHART INCREASING
    const std::string CLOCK      = "\xE2\x8F\xB0";      // ⏰ U+23F0 ALARM CLOCK
    const std::string FIRE       = "\xF0\x9F\x94\xA5";  // 🔥 U+1F525 FIRE
    const std::string SLOT_MACHINE = "\xF0\x9F\x8E\xB0";  // 🎰 U+1F3B0 SLOT MACHINE
    const std::string FERRIS_WHEEL = "\xF0\x9F\x8E\xA1";  // 🎡 U+1F3A1 FERRIS WHEEL
    const std::string PLAYING_CARD = "\xF0\x9F\x83\x8F";  // 🃏 U+1F0CF PLAYING CARD BLACK JOKER
    const std::string CONVENIENCE_STORE = "\xF0\x9F\x8F\xAA";  // 🏪 U+1F3EA CONVENIENCE STORE
    const std::string DOLLAR     = "\xF0\x9F\x92\xB5";  // 💵 U+1F4B5 DOLLAR BANKNOTE
    const std::string FLOPPY     = "\xF0\x9F\x92\xBE";  // 💾 U+1F4BE FLOPPY DISK
    const std::string DOOR       = "\xF0\x9F\x9A\xAA";  // 🚪 U+1F6AA DOOR
    const std::string CROWN      = "\xF0\x9F\x91\x91";  // 👑 U+1F451 CROWN
    const std::string GLOBE      = "\xF0\x9F\x8C\x8D";  // 🌍 U+1F30D EARTH GLOBE EUROPE-AFRICA
    const std::string JAR        = "\xF0\x9F\xAB\x99";  // 🫙 U+1FAD9 JAR
    const std::string BUILDING   = "\xF0\x9F\x8F\x9B";  // 🏛 U+1F3DB CLASSICAL BUILDING
    const std::string UP_ARROW   = "\xE2\xAC\x86\xEF\xB8\x8F";  // ⬆️ UP ARROW

    // ========================================================================
    // ASCII FALLBACKS (for terminals that don't support Unicode)
    // ========================================================================
    namespace ASCII {
        const std::string H_LINE    = "-";
        const std::string V_LINE    = "|";
        const std::string TL_CORNER = "+";
        const std::string TR_CORNER = "+";
        const std::string BL_CORNER = "+";
        const std::string BR_CORNER = "+";
        const std::string SPADE     = "S";
        const std::string HEART     = "H";
        const std::string DIAMOND   = "D";
        const std::string CLUB      = "C";
        const std::string STAR      = "*";
        const std::string CHECK     = "v";
        const std::string CROSS_X   = "x";
    }
}

#endif // UNICODE_H
