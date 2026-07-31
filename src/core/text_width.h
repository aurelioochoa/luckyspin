/*
 * text_width.h - Terminal display width of UTF-8 strings
 *
 * Why this exists:
 *   std::string::length() returns BYTES. A terminal lays out COLUMNS.
 *   "♠" is 3 bytes but 1 column; "🎲" is 4 bytes but 2 columns; a color
 *   escape like "\033[32m" is 5 bytes and 0 columns. Using length() for
 *   alignment therefore shifts any line containing Unicode or color.
 *
 * displayWidth() decodes UTF-8, skips ANSI escape sequences, and sums the
 * per-codepoint column count using a wcwidth-style table.
 *
 * This header is intentionally free of terminal/OS dependencies so it can be
 * unit tested directly.
 */

#ifndef CORE_TEXT_WIDTH_H
#define CORE_TEXT_WIDTH_H

#include <string>
#include <cstddef>

namespace text {

typedef unsigned int codepoint;

// A half-open [lo, hi] inclusive range table entry.
struct Range { codepoint lo; codepoint hi; };

inline bool inRanges(codepoint cp, const Range* table, std::size_t count) {
    // Ranges are sorted, so binary search.
    std::size_t low = 0, high = count;
    while (low < high) {
        std::size_t mid = low + (high - low) / 2;
        if (cp < table[mid].lo)      high = mid;
        else if (cp > table[mid].hi) low = mid + 1;
        else                         return true;
    }
    return false;
}

// Codepoints that occupy no columns: combining marks, variation selectors,
// zero-width spaces/joiners, and enclosing keycap marks.
inline bool isZeroWidth(codepoint cp) {
    static const Range kZero[] = {
        {0x0300, 0x036F},   // Combining Diacritical Marks
        {0x0483, 0x0489},
        {0x0591, 0x05BD},
        {0x0610, 0x061A},
        {0x064B, 0x065F},
        {0x0670, 0x0670},
        {0x06D6, 0x06DC},
        {0x0E31, 0x0E31},
        {0x0E34, 0x0E3A},
        {0x1AB0, 0x1AFF},
        {0x1DC0, 0x1DFF},
        {0x200B, 0x200F},   // ZWSP, ZWNJ, ZWJ, LRM, RLM
        {0x2028, 0x202E},
        {0x2060, 0x206F},
        {0x20D0, 0x20F0},   // Combining Diacritical Marks for Symbols (incl. U+20E3 keycap)
        {0xFE00, 0xFE0F},   // Variation Selectors 1-16
        {0xFE20, 0xFE2F},
        {0xFEFF, 0xFEFF},   // BOM
        {0xE0100, 0xE01EF}, // Variation Selectors Supplement
    };
    return inRanges(cp, kZero, sizeof(kZero) / sizeof(kZero[0]));
}

// Codepoints that occupy two columns: East Asian Wide/Fullwidth plus the
// emoji blocks whose default presentation is emoji.
inline bool isWide(codepoint cp) {
    static const Range kWide[] = {
        {0x1100, 0x115F},   // Hangul Jamo initial consonants
        {0x231A, 0x231B},   // watch, hourglass
        {0x2329, 0x232A},
        {0x23E9, 0x23EC},
        {0x23F0, 0x23F0},   // alarm clock
        {0x23F3, 0x23F3},
        {0x25FD, 0x25FE},
        {0x2614, 0x2615},
        {0x2648, 0x2653},   // zodiac
        {0x267F, 0x267F},
        {0x2693, 0x2693},
        {0x26A1, 0x26A1},
        {0x26AA, 0x26AB},
        {0x26BD, 0x26BE},
        {0x26C4, 0x26C5},
        {0x26CE, 0x26CE},
        {0x26D4, 0x26D4},
        {0x26EA, 0x26EA},
        {0x26F2, 0x26F3},
        {0x26F5, 0x26F5},
        {0x26FA, 0x26FA},
        {0x26FD, 0x26FD},
        {0x2705, 0x2705},
        {0x270A, 0x270B},
        {0x2728, 0x2728},
        {0x274C, 0x274C},
        {0x274E, 0x274E},
        {0x2753, 0x2755},
        {0x2757, 0x2757},
        {0x2795, 0x2797},
        {0x27B0, 0x27B0},
        {0x27BF, 0x27BF},
        {0x2B1B, 0x2B1C},
        {0x2B50, 0x2B50},   // white medium star
        {0x2B55, 0x2B55},
        {0x2E80, 0x303E},   // CJK radicals, Kangxi, CJK symbols
        {0x3041, 0x33FF},   // Hiragana .. CJK compatibility
        {0x3400, 0x4DBF},   // CJK Ext A
        {0x4E00, 0x9FFF},   // CJK Unified Ideographs
        {0xA000, 0xA4CF},   // Yi
        {0xAC00, 0xD7A3},   // Hangul syllables
        {0xF900, 0xFAFF},   // CJK compatibility ideographs
        {0xFE10, 0xFE19},
        {0xFE30, 0xFE6F},
        {0xFF00, 0xFF60},   // Fullwidth forms
        {0xFFE0, 0xFFE6},
        {0x16FE0, 0x16FE4},
        {0x17000, 0x187F7},
        {0x1F000, 0x1F0FF}, // Mahjong, dominoes, playing cards (incl. U+1F0CF joker)
        {0x1F18E, 0x1F18E},
        {0x1F191, 0x1F19A},
        {0x1F200, 0x1F320},
        {0x1F32D, 0x1F335},
        {0x1F337, 0x1F37C},
        {0x1F37E, 0x1F393},
        {0x1F3A0, 0x1F3CA}, // incl. 🎠 slot/wheel/dice range
        {0x1F3CF, 0x1F3D3},
        {0x1F3E0, 0x1F3F0}, // incl. 🏪 convenience store
        {0x1F3F4, 0x1F3F4},
        {0x1F3F8, 0x1F43E},
        {0x1F440, 0x1F440},
        {0x1F442, 0x1F4FC}, // incl. 💰 💵 💾 💎 📈
        {0x1F4FF, 0x1F53D},
        {0x1F54B, 0x1F54E},
        {0x1F550, 0x1F567},
        {0x1F57A, 0x1F57A},
        {0x1F595, 0x1F596},
        {0x1F5A4, 0x1F5A4},
        {0x1F5FB, 0x1F64F}, // incl. 🌍 emoticons
        {0x1F680, 0x1F6C5}, // incl. 🚪 door
        {0x1F6CC, 0x1F6CC},
        {0x1F6D0, 0x1F6D2},
        {0x1F6D5, 0x1F6D7},
        {0x1F6EB, 0x1F6EC},
        {0x1F6F4, 0x1F6FC},
        {0x1F7E0, 0x1F7EB},
        {0x1F90C, 0x1F93A},
        {0x1F93C, 0x1F945},
        {0x1F947, 0x1F978},
        {0x1F97A, 0x1F9CB},
        {0x1F9CD, 0x1F9FF},
        {0x1FA70, 0x1FAFF}, // incl. 🫙 U+1FAD9 jar
        {0x20000, 0x2FFFD}, // CJK Ext B+
        {0x30000, 0x3FFFD},
    };
    return inRanges(cp, kWide, sizeof(kWide) / sizeof(kWide[0]));
}

/*
 * decodeUtf8() - Read one codepoint starting at bytes[i], advancing i.
 *
 * Malformed sequences consume a single byte and yield U+FFFD so that a bad
 * byte can never cause an infinite loop or an out-of-bounds read.
 */
inline codepoint decodeUtf8(const std::string& s, std::size_t& i) {
    const unsigned char c = static_cast<unsigned char>(s[i]);
    std::size_t remaining = s.size() - i;

    if (c < 0x80) { i += 1; return c; }

    int len;
    codepoint cp;
    if ((c & 0xE0) == 0xC0)      { len = 2; cp = c & 0x1F; }
    else if ((c & 0xF0) == 0xE0) { len = 3; cp = c & 0x0F; }
    else if ((c & 0xF8) == 0xF0) { len = 4; cp = c & 0x07; }
    else                         { i += 1; return 0xFFFD; }  // stray continuation byte

    if (remaining < static_cast<std::size_t>(len)) { i += 1; return 0xFFFD; }

    for (int k = 1; k < len; ++k) {
        const unsigned char cc = static_cast<unsigned char>(s[i + k]);
        if ((cc & 0xC0) != 0x80) { i += 1; return 0xFFFD; }  // truncated sequence
        cp = (cp << 6) | (cc & 0x3F);
    }
    i += len;
    return cp;
}

/*
 * displayWidth() - Number of terminal columns a string occupies.
 *
 * Handles:
 *   - ANSI CSI escape sequences (ESC [ ... final), which cost 0 columns
 *   - Other ESC-prefixed 2-byte sequences
 *   - Wide (2-column) and zero-width codepoints
 *   - U+FE0F VARIATION SELECTOR-16, which promotes a 1-column base to 2
 */
inline int displayWidth(const std::string& s) {
    int width = 0;
    int lastBaseWidth = 0;
    std::size_t i = 0;

    while (i < s.size()) {
        const unsigned char c = static_cast<unsigned char>(s[i]);

        // ANSI escape sequence: consume without counting columns.
        if (c == 0x1B) {
            ++i;
            if (i < s.size() && s[i] == '[') {
                ++i;
                // CSI parameters/intermediates, terminated by a byte in @..~
                while (i < s.size()) {
                    const unsigned char f = static_cast<unsigned char>(s[i]);
                    ++i;
                    if (f >= 0x40 && f <= 0x7E) break;
                }
            } else if (i < s.size()) {
                ++i;  // two-byte escape such as ESC(B
            }
            lastBaseWidth = 0;
            continue;
        }

        const codepoint cp = decodeUtf8(s, i);

        if (cp == 0xFE0F) {
            // Emoji presentation selector: widen the preceding base character.
            if (lastBaseWidth == 1) { width += 1; lastBaseWidth = 2; }
            continue;
        }
        if (isZeroWidth(cp)) continue;
        if (cp == '\t') { width += 8; lastBaseWidth = 8; continue; }
        if (cp < 0x20)  { lastBaseWidth = 0; continue; }  // other control chars

        const int w = isWide(cp) ? 2 : 1;
        width += w;
        lastBaseWidth = w;
    }

    return width;
}

/*
 * padTo() - Spaces needed to pad a string out to a target column count.
 *
 * Returns 0 (never negative) when the string is already at or past the target,
 * avoiding the signed/unsigned trap of `width - s.length()`.
 */
inline int padTo(const std::string& s, int targetColumns) {
    const int w = displayWidth(s);
    return (targetColumns > w) ? (targetColumns - w) : 0;
}

} // namespace text

#endif // CORE_TEXT_WIDTH_H
