/*
 * symbols.h - Game-facing symbol sets for Lucky Spin Casino
 *
 * The raw codepoints live in unicode.h. This header groups them into the
 * three sets the game actually renders with, so that a codepoint is defined
 * exactly once in the project:
 *
 *   Symbols:: - slot machine reel faces
 *   Cards::   - playing card suits (+ their colors)
 *   UI::      - box drawing, arrows, and menu icons
 *
 * These are aliases, not new literals. Changing a codepoint in unicode.h
 * changes it everywhere it is used.
 */

#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <string>
#include "colors.h"
#include "unicode.h"

// ============================================================================
// SLOT MACHINE REEL SYMBOLS
// ============================================================================
namespace Symbols {
    const std::string CHERRY  = Unicode::CHERRY;
    const std::string ORANGE  = Unicode::ORANGE;
    const std::string LEMON   = Unicode::LEMON;
    const std::string GRAPE   = Unicode::GRAPE;
    const std::string BELL    = Unicode::BELL;
    const std::string SEVEN   = Unicode::SEVEN;
    const std::string DIAMOND = Unicode::GEM;
    const std::string STAR    = Unicode::STAR_EMOJI;

    // Array of all reel faces, for random selection
    const std::string ALL[] = {CHERRY, ORANGE, LEMON, GRAPE, BELL, SEVEN, DIAMOND, STAR};

    // Accent color for each reel face (parallel to ALL)
    const std::string COLORS[] = {
        Color::RED,      // Cherry
        Color::YELLOW,   // Orange
        Color::YELLOW,   // Lemon
        Color::MAGENTA,  // Grape
        Color::CYAN,     // Bell
        Color::GREEN,    // Seven (lucky!)
        Color::CYAN,     // Diamond
        Color::YELLOW    // Star
    };

    // Total number of distinct reel faces
    const int COUNT = sizeof(ALL) / sizeof(ALL[0]);
}

// ============================================================================
// PLAYING CARD SUITS
// ============================================================================
namespace Cards {
    const std::string SPADE   = Unicode::SPADE;
    const std::string HEART   = Unicode::HEART;
    const std::string DIAMOND = Unicode::DIAMOND;
    const std::string CLUB    = Unicode::CLUB;

    // Suit colors: hearts and diamonds are red, spades and clubs are black/white
    const std::string SPADE_COLOR   = Color::WHITE;
    const std::string HEART_COLOR   = Color::RED;
    const std::string DIAMOND_COLOR = Color::RED;
    const std::string CLUB_COLOR    = Color::WHITE;

    // Suit lookup, ordered to match SUIT_COLORS below
    const std::string SUITS[]       = {SPADE, HEART, DIAMOND, CLUB};
    const std::string SUIT_COLORS[] = {SPADE_COLOR, HEART_COLOR, DIAMOND_COLOR, CLUB_COLOR};
    const int SUIT_COUNT = 4;
}

// ============================================================================
// UI DECORATIONS
// ============================================================================
namespace UI {
    // Box drawing (U+2500 block)
    const std::string H_LINE    = Unicode::H_LINE;
    const std::string V_LINE    = Unicode::V_LINE;
    const std::string TL_CORNER = Unicode::TL_CORNER;
    const std::string TR_CORNER = Unicode::TR_CORNER;
    const std::string BL_CORNER = Unicode::BL_CORNER;
    const std::string BR_CORNER = Unicode::BR_CORNER;
    const std::string T_JOIN    = Unicode::T_JOIN;
    const std::string B_JOIN    = Unicode::B_JOIN;
    const std::string L_JOIN    = Unicode::L_JOIN;
    const std::string R_JOIN    = Unicode::R_JOIN;
    const std::string CROSS     = Unicode::CROSS;

    // Stars (U+2605 block)
    const std::string BLACK_STAR = Unicode::BLACK_STAR;
    const std::string WHITE_STAR = Unicode::WHITE_STAR;

    // Arrows / triangles (U+25xx block)
    const std::string ARROW_RIGHT = Unicode::ARROW_RIGHT;
    const std::string ARROW_UP    = Unicode::ARROW_UP;
    const std::string ARROW_DOWN  = Unicode::ARROW_DOWN;
    const std::string ARROW_LEFT  = Unicode::ARROW_LEFT;

    // Check marks
    const std::string CHECK   = Unicode::CHECK;
    const std::string CROSS_X = Unicode::CROSS_X;

    // Emoji icons
    const std::string MONEY    = Unicode::MONEY_BAG;
    const std::string TROPHY   = Unicode::TROPHY;
    const std::string DICE     = Unicode::DICE;
    const std::string CHART    = Unicode::CHART;
    const std::string CLOCK    = Unicode::CLOCK;
    const std::string STAR     = Unicode::STAR_EMOJI;
    const std::string FIRE     = Unicode::FIRE;
    const std::string SLOT     = Unicode::SLOT_MACHINE;
    const std::string WHEEL    = Unicode::FERRIS_WHEEL;
    const std::string CARD     = Unicode::PLAYING_CARD;
    const std::string STORE    = Unicode::CONVENIENCE_STORE;
    const std::string DOLLAR   = Unicode::DOLLAR;
    const std::string FLOPPY   = Unicode::FLOPPY;
    const std::string DOOR     = Unicode::DOOR;
    const std::string CROWN    = Unicode::CROWN;
    const std::string GLOBE    = Unicode::GLOBE;
    const std::string TARGET   = Unicode::TARGET;
    const std::string REFRESH  = Unicode::REFRESH;
    const std::string GEM      = Unicode::GEM;
    const std::string JAR      = Unicode::JAR;
    const std::string BUILDING = Unicode::BUILDING;
    const std::string UP_ARROW = Unicode::UP_ARROW_PLAIN;
}

#endif // SYMBOLS_H
