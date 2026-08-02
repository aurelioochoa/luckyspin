/*
 * banner.h - The animated casino marquee header
 *
 * rows() builds the five header lines as pure text, so a test can assert what
 * the terminal cannot: every row must span exactly the same number of display
 * columns.
 *
 * That used not to be true. The title row was hand-padded with eight spaces
 * on each side, which assumed the 🎰 glyphs were one column wide; they render
 * as two, so the title row came out 7 columns short and its right border did
 * not line up with the rows above and below it.
 *
 * displayHeader() in gambling_game.cpp just prints what this returns.
 */

#ifndef CORE_BANNER_H
#define CORE_BANNER_H

#include <string>
#include <vector>

#include "text_width.h"
#include "../symbols.h"

namespace banner {

// Columns between the two vertical borders.
const int INTERIOR_WIDTH = 46;

/*
 * BannerColors - ANSI escapes applied around the glyphs.
 *
 * Escape sequences are zero columns wide, so they never affect alignment.
 * Defaults are uncolored, which is what the tests use.
 */
struct Colors {
    std::string border;   // box drawing
    std::string star;     // the marquee stars (cycles each frame)
    std::string title;    // the title text
    std::string reset;    // emitted after each colored span
};

// Join four star glyphs with single spaces: "s1 s2 s3 s4".
inline std::string starSeq(const std::string& a, const std::string& b,
                           const std::string& c, const std::string& d) {
    return a + " " + b + " " + c + " " + d;
}

// Pad content to exactly `width` columns, splitting any odd remainder right.
inline std::string centerIn(const std::string& content, int width) {
    const int pad = width - text::displayWidth(content);
    if (pad <= 0) return content;
    return std::string(pad / 2, ' ') + content + std::string(pad - pad / 2, ' ');
}

/*
 * starRow() - One marquee row: "│ s1 s2 s3 s4  ...  s4 s3 s2 s1 │"
 *
 * The padding between the two clusters is computed from display columns, so
 * the row is exactly INTERIOR_WIDTH wide no matter what glyphs are passed in.
 * `mirrored` swaps the two clusters, which is what makes the bottom star row
 * the mirror image of the top one.
 */
inline std::string starRow(const std::string stars[4], bool mirrored,
                           const Colors& c) {
    std::string left  = starSeq(stars[0], stars[1], stars[2], stars[3]);
    std::string right = starSeq(stars[3], stars[2], stars[1], stars[0]);
    if (mirrored) std::swap(left, right);

    // One space against each border, the rest of the padding in the middle
    const int pad = INTERIOR_WIDTH - 2
                  - text::displayWidth(left) - text::displayWidth(right);
    const std::string gap(pad > 0 ? pad : 0, ' ');

    return c.border + UI::V_LINE + c.reset
         + c.star + " " + left + gap + right + " " + c.reset
         + c.border + UI::V_LINE + c.reset;
}

/*
 * rows() - The five header lines for a marquee animation frame.
 *
 * Each frame lights one star in sequence, like bulbs chasing around a casino
 * sign. Every returned row has the same display width; the unit tests assert
 * that for all four frames.
 */
inline std::vector<std::string> rows(int frame, const Colors& c) {
    std::vector<std::string> out;

    std::string top = c.border + UI::TL_CORNER;
    for (int i = 0; i < INTERIOR_WIDTH; ++i) top += UI::H_LINE;
    top += UI::TR_CORNER + c.reset;

    std::string bottom = c.border + UI::BL_CORNER;
    for (int i = 0; i < INTERIOR_WIDTH; ++i) bottom += UI::H_LINE;
    bottom += UI::BR_CORNER + c.reset;

    // One star per frame, in sequence
    std::string stars[4];
    for (int i = 0; i < 4; ++i) {
        stars[i] = (frame % 4 == i) ? UI::BLACK_STAR : UI::WHITE_STAR;
    }

    const std::string title = c.title + UI::SLOT + " LUCKY SPIN CASINO "
                            + UI::SLOT + c.reset;
    const std::string titleRow = c.border + UI::V_LINE + c.reset
                               + centerIn(title, INTERIOR_WIDTH)
                               + c.border + UI::V_LINE + c.reset;

    out.push_back(top);
    out.push_back(starRow(stars, false, c));
    out.push_back(titleRow);
    out.push_back(starRow(stars, true, c));
    out.push_back(bottom);
    return out;
}

} // namespace banner

#endif // CORE_BANNER_H
