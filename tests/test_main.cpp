/*
 * test_main.cpp - Unit tests for Lucky Spin Casino core logic
 *
 * Covers the pure, headless parts of the game in src/core/:
 *   - text_width.h : terminal column accounting for UTF-8 + ANSI
 *   - money.h      : large-number currency formatting
 *   - rng.h        : distribution and fractional probability handling
 *   - deck.h       : card values, hand scoring, shoe integrity
 *   - save_path.h  : XDG path resolution
 *
 * Deliberately dependency-free (no gtest) so `make test` works on a bare
 * toolchain. Run with: make test
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdlib>
#include <cmath>

#include "core/text_width.h"
#include "core/money.h"
#include "core/rng.h"
#include "core/deck.h"
#include "core/save_path.h"

// ============================================================================
// MINIMAL TEST HARNESS
// ============================================================================

static int g_checks = 0;
static int g_failures = 0;
static std::string g_currentSuite;

static void suite(const std::string& name) {
    g_currentSuite = name;
    std::cout << "\n\033[1;36m" << name << "\033[0m\n";
}

static void check(bool condition, const std::string& what) {
    ++g_checks;
    if (condition) {
        std::cout << "  \033[32mPASS\033[0m " << what << "\n";
    } else {
        ++g_failures;
        std::cout << "  \033[31mFAIL\033[0m " << what << "\n";
    }
}

template <typename T>
static void checkEqual(const T& actual, const T& expected, const std::string& what) {
    ++g_checks;
    if (actual == expected) {
        std::cout << "  \033[32mPASS\033[0m " << what << "\n";
    } else {
        ++g_failures;
        std::cout << "  \033[31mFAIL\033[0m " << what
                  << "  (expected \"" << expected << "\", got \"" << actual << "\")\n";
    }
}

static void checkNear(double actual, double expected, double tolerance,
                      const std::string& what) {
    ++g_checks;
    if (std::fabs(actual - expected) <= tolerance) {
        std::cout << "  \033[32mPASS\033[0m " << what << "\n";
    } else {
        ++g_failures;
        std::cout << "  \033[31mFAIL\033[0m " << what
                  << "  (expected " << expected << " +/-" << tolerance
                  << ", got " << actual << ")\n";
    }
}

// ============================================================================
// TEXT WIDTH
// ============================================================================

static void testTextWidth() {
    suite("core/text_width.h - display width");

    checkEqual(text::displayWidth(""), 0, "empty string is 0 columns");
    checkEqual(text::displayWidth("BLACKJACK"), 9, "ASCII width equals length");

    // Card suits are 3 UTF-8 bytes but occupy 1 column each.
    const std::string spade = "\xE2\x99\xA0";
    checkEqual((int)spade.size(), 3, "spade is 3 bytes");
    checkEqual(text::displayWidth(spade), 1, "spade is 1 column");

    // Emoji are 4 bytes but occupy 2 columns.
    const std::string dice = "\xF0\x9F\x8E\xB2";
    checkEqual((int)dice.size(), 4, "dice emoji is 4 bytes");
    checkEqual(text::displayWidth(dice), 2, "dice emoji is 2 columns");

    // The exact title that used to mis-center.
    const std::string title = spade + " BLACKJACK " + "\xE2\x99\xA5";
    checkEqual((int)title.size(), 17, "blackjack title is 17 bytes");
    checkEqual(text::displayWidth(title), 13, "blackjack title is 13 columns");

    // ANSI escapes must not consume columns.
    checkEqual(text::displayWidth("\033[32mgreen\033[0m"), 5,
               "ANSI color codes are 0 columns");
    checkEqual(text::displayWidth("\033[1;33;44mX\033[0m"), 1,
               "multi-parameter CSI is 0 columns");

    // Variation selector 16 promotes a narrow base glyph to 2 columns.
    const std::string upArrowPlain = "\xE2\xAC\x86";
    const std::string upArrowEmoji = "\xE2\xAC\x86\xEF\xB8\x8F";
    checkEqual(text::displayWidth(upArrowPlain), 1, "bare up arrow is 1 column");
    checkEqual(text::displayWidth(upArrowEmoji), 2, "up arrow + VS16 is 2 columns");

    // Keycap sequence: '7' + VS16 + U+20E3 renders as one 2-column glyph.
    const std::string seven = "7\xEF\xB8\x8F\xE2\x83\xA3";
    checkEqual(text::displayWidth(seven), 2, "keycap seven is 2 columns");

    // Wide star used throughout the UI.
    checkEqual(text::displayWidth("\xE2\xAD\x90"), 2, "star emoji is 2 columns");

    // Malformed UTF-8 must terminate and not hang or overrun. Each byte that
    // cannot start a valid sequence is consumed as one replacement character.
    checkEqual(text::displayWidth("\xFF\xFE"), 2, "invalid bytes count as replacement chars");
    checkEqual(text::displayWidth("\xE2\x99"), 2, "truncated sequence does not overrun");
    checkEqual(text::displayWidth("\xE2\x99\xA0" "A"), 2, "valid glyph then ASCII");

    // padTo must never go negative (the old signed/unsigned bug).
    checkEqual(text::padTo("abc", 10), 7, "padTo pads short strings");
    checkEqual(text::padTo("abcdefghijkl", 5), 0, "padTo clamps at 0, never negative");
    checkEqual(text::padTo(dice, 10), 8, "padTo accounts for wide glyphs");
}

// ============================================================================
// MONEY FORMATTING
// ============================================================================

static void testMoney() {
    suite("core/money.h - currency formatting");

    checkEqual(formatMoney(0.0), std::string("$0.00"), "zero");
    checkEqual(formatMoney(99.5), std::string("$99.50"), "under 1K is plain");
    checkEqual(formatMoney(999.99), std::string("$999.99"), "just under 1K");
    checkEqual(formatMoney(1000.0), std::string("$1.00K"), "1K boundary");
    checkEqual(formatMoney(1234.0), std::string("$1.23K"), "thousands");
    checkEqual(formatMoney(1.5e6), std::string("$1.50M"), "millions");
    checkEqual(formatMoney(4.33e9), std::string("$4.33B"), "billions");
    checkEqual(formatMoney(1e12), std::string("$1.00T"), "trillions");

    // These are the cases the old implementation rendered as "1000000.00T".
    checkEqual(formatMoney(1e15), std::string("$1.00Qa"), "quadrillions");
    checkEqual(formatMoney(1e18), std::string("$1.00Qi"), "quintillions");
    checkEqual(formatMoney(1e21), std::string("$1.00Sx"), "sextillions");
    checkEqual(formatMoney(1e33), std::string("$1.00Dc"), "decillions");

    // Beyond the suffix table, fall back to scientific notation.
    check(formatMoney(1e45).find('e') != std::string::npos,
          "very large values use scientific notation");

    checkEqual(formatMoney(-25.0), std::string("-$25.00"), "negative amounts");

    // NaN/inf must never reach the UI as "nan".
    const double nan = std::sqrt(-1.0);
    checkEqual(formatMoney(nan), std::string("$0.00"), "NaN renders as $0.00");
}

// ============================================================================
// RNG
// ============================================================================

static void testRng() {
    suite("core/rng.h - random number generation");

    // Same seed must produce the same stream (this is what makes tests possible).
    Rng a(12345), b(12345);
    bool identical = true;
    for (int i = 0; i < 100; ++i) {
        if (a.below(1000) != b.below(1000)) { identical = false; break; }
    }
    check(identical, "same seed produces identical sequences");

    Rng c(1), d(2);
    bool differs = false;
    for (int i = 0; i < 100; ++i) {
        if (c.below(1000) != d.below(1000)) { differs = true; break; }
    }
    check(differs, "different seeds produce different sequences");

    // below() must stay in range and cover it.
    Rng rng(42);
    bool inRange = true;
    std::map<int, int> seen;
    for (int i = 0; i < 20000; ++i) {
        const int v = rng.below(37);
        if (v < 0 || v > 36) { inRange = false; break; }
        seen[v]++;
    }
    check(inRange, "below(37) stays within [0, 36]");
    checkEqual((int)seen.size(), 37, "below(37) covers every value");

    checkEqual(rng.below(0), 0, "below(0) is 0");
    checkEqual(rng.below(1), 0, "below(1) is 0");

    // range() is inclusive on both ends.
    bool rangeOk = true;
    for (int i = 0; i < 5000; ++i) {
        const int v = rng.range(5, 10);
        if (v < 5 || v > 10) { rangeOk = false; break; }
    }
    check(rangeOk, "range(5,10) stays within bounds");

    // chancePercent must honour fractional percentages.
    // The old `rand() % 100 < 62.5` paid out 63% of the time.
    {
        Rng r(7);
        int hits = 0;
        const int trials = 200000;
        for (int i = 0; i < trials; ++i) {
            if (r.chancePercent(62.5)) ++hits;
        }
        checkNear(100.0 * hits / trials, 62.5, 0.5,
                  "chancePercent(62.5) fires ~62.5% of the time");
    }
    {
        Rng r(8);
        int hits = 0;
        const int trials = 100000;
        for (int i = 0; i < trials; ++i) {
            if (r.chancePercent(25.0)) ++hits;
        }
        checkNear(100.0 * hits / trials, 25.0, 0.5,
                  "chancePercent(25) fires ~25% of the time");
    }

    Rng edge(9);
    check(!edge.chancePercent(0.0), "chancePercent(0) never fires");
    check(edge.chancePercent(100.0), "chancePercent(100) always fires");
    check(!edge.chancePercent(-5.0), "negative chance never fires");

    // unit() must stay in [0, 1).
    bool unitOk = true;
    for (int i = 0; i < 10000; ++i) {
        const double u = edge.unit();
        if (u < 0.0 || u >= 1.0) { unitOk = false; break; }
    }
    check(unitOk, "unit() stays within [0, 1)");
}

// ============================================================================
// DECK
// ============================================================================

static void testDeck() {
    suite("core/deck.h - cards and shoe");

    // Rank names: the J/Q/K cases the old code could never reach.
    checkEqual(cards::rankName(1),  std::string("A"),  "rank 1 is A");
    checkEqual(cards::rankName(2),  std::string("2"),  "rank 2 is 2");
    checkEqual(cards::rankName(10), std::string("10"), "rank 10 is 10");
    checkEqual(cards::rankName(11), std::string("J"),  "rank 11 is J");
    checkEqual(cards::rankName(12), std::string("Q"),  "rank 12 is Q");
    checkEqual(cards::rankName(13), std::string("K"),  "rank 13 is K");

    // Face cards are worth 10 but keep distinct ranks.
    checkEqual(cards::blackjackValue(11), 10, "jack is worth 10");
    checkEqual(cards::blackjackValue(13), 10, "king is worth 10");
    checkEqual(cards::blackjackValue(1),  1,  "ace base value is 1");

    checkEqual(cards::isRedSuit(cards::HEARTS),   true,  "hearts are red");
    checkEqual(cards::isRedSuit(cards::DIAMONDS), true,  "diamonds are red");
    checkEqual(cards::isRedSuit(cards::SPADES),   false, "spades are not red");
    checkEqual(cards::isRedSuit(cards::CLUBS),    false, "clubs are not red");

    // Hand scoring, including ace promotion.
    {
        std::vector<cards::Card> hand;
        hand.push_back(cards::Card(1, 0));    // A
        hand.push_back(cards::Card(13, 1));   // K
        checkEqual(cards::handTotal(hand), 21, "A+K is 21");
        check(cards::isNaturalBlackjack(hand), "A+K is a natural blackjack");
        check(cards::isSoft(hand), "A+K is soft");
    }
    {
        std::vector<cards::Card> hand;
        hand.push_back(cards::Card(1, 0));
        hand.push_back(cards::Card(1, 1));
        checkEqual(cards::handTotal(hand), 12, "A+A is 12, not 22");
    }
    {
        std::vector<cards::Card> hand;
        hand.push_back(cards::Card(1, 0));
        hand.push_back(cards::Card(1, 1));
        hand.push_back(cards::Card(9, 2));
        checkEqual(cards::handTotal(hand), 21, "A+A+9 is 21");
    }
    {
        std::vector<cards::Card> hand;
        hand.push_back(cards::Card(10, 0));
        hand.push_back(cards::Card(5, 1));
        hand.push_back(cards::Card(9, 2));
        checkEqual(cards::handTotal(hand), 24, "10+5+9 busts at 24");
        check(!cards::isSoft(hand), "busted hard hand is not soft");
    }
    {
        std::vector<cards::Card> hand;
        hand.push_back(cards::Card(1, 0));
        hand.push_back(cards::Card(5, 1));
        hand.push_back(cards::Card(10, 2));
        checkEqual(cards::handTotal(hand), 16, "A+5+10 demotes ace to 1");
    }
    {
        // 21 on three cards is 21 but NOT a natural.
        std::vector<cards::Card> hand;
        hand.push_back(cards::Card(7, 0));
        hand.push_back(cards::Card(7, 1));
        hand.push_back(cards::Card(7, 2));
        checkEqual(cards::handTotal(hand), 21, "7+7+7 is 21");
        check(!cards::isNaturalBlackjack(hand), "three-card 21 is not a natural");
    }

    // Shoe composition.
    cards::Shoe shoe(6);
    checkEqual((int)shoe.size(), 312, "6-deck shoe holds 312 cards");
    checkEqual((int)shoe.remaining(), 312, "fresh shoe is full");

    Rng rng(2024);
    shoe.shuffle(rng);
    checkEqual((int)shoe.remaining(), 312, "shuffle refills the shoe");

    // Deal the whole shoe and verify exact composition: every rank appears
    // 24 times (6 decks x 4 suits) and every suit 78 times.
    std::map<int, int> rankCounts;
    std::map<int, int> suitCounts;
    for (int i = 0; i < 312; ++i) {
        const cards::Card card = shoe.draw(rng);
        rankCounts[card.rank]++;
        suitCounts[card.suit]++;
    }
    checkEqual((int)rankCounts.size(), 13, "all 13 ranks present");
    checkEqual((int)suitCounts.size(), 4,  "all 4 suits present");

    bool rankBalance = true;
    for (int r = 1; r <= 13; ++r) {
        if (rankCounts[r] != 24) { rankBalance = false; break; }
    }
    check(rankBalance, "each rank appears exactly 24 times");

    bool suitBalance = true;
    for (int s = 0; s < 4; ++s) {
        if (suitCounts[s] != 78) { suitBalance = false; break; }
    }
    check(suitBalance, "each suit appears exactly 78 times");

    // Drawing past the end reshuffles rather than reading out of bounds.
    const cards::Card wrapped = shoe.draw(rng);
    check(wrapped.rank >= 1 && wrapped.rank <= 13,
          "drawing an exhausted shoe reshuffles and returns a valid card");

    // Shuffling must actually permute (a fresh ordered shoe is not shuffled).
    cards::Shoe ordered(1);
    cards::Shoe shuffled(1);
    Rng r2(99);
    shuffled.shuffle(r2);
    int differences = 0;
    for (int i = 0; i < 52; ++i) {
        const cards::Card o = ordered.draw(r2);
        const cards::Card s = shuffled.draw(r2);
        if (o.rank != s.rank || o.suit != s.suit) ++differences;
    }
    check(differences > 20, "shuffle meaningfully reorders the deck");

    // Penetration triggers a reshuffle before the shoe is exhausted.
    cards::Shoe pen(6);
    Rng r3(5);
    pen.shuffle(r3);
    for (int i = 0; i < 235; ++i) pen.draw(r3);
    check(pen.needsShuffle(), "needsShuffle() is true past ~75% penetration");
}

// ============================================================================
// SAVE PATH
// ============================================================================

static void testSavePath() {
    suite("core/save_path.h - XDG save location");

    checkEqual(savepath::joinPath("/a", "b"),  std::string("/a/b"),  "join adds a separator");
    checkEqual(savepath::joinPath("/a/", "b"), std::string("/a/b"),  "join avoids a double separator");
    checkEqual(savepath::joinPath("", "b"),    std::string("b"),     "join handles an empty base");

    // XDG_DATA_HOME takes precedence when set.
    setenv("XDG_DATA_HOME", "/tmp/luckyspin-test-xdg", 1);
    checkEqual(savepath::dataDir(), std::string("/tmp/luckyspin-test-xdg/luckyspin"),
               "XDG_DATA_HOME is honoured");

    // Falls back to ~/.local/share when XDG_DATA_HOME is absent.
    unsetenv("XDG_DATA_HOME");
    setenv("HOME", "/tmp/luckyspin-test-home", 1);
    checkEqual(savepath::dataDir(), std::string("/tmp/luckyspin-test-home/.local/share/luckyspin"),
               "falls back to ~/.local/share");

    // ensureDir creates nested directories.
    const std::string nested = "/tmp/luckyspin-test-home/.local/share/luckyspin";
    check(savepath::ensureDir(nested), "ensureDir creates nested directories");
    check(savepath::ensureDir(nested), "ensureDir is idempotent");

    // resolve() lands inside the data directory.
    const std::string resolved = savepath::resolve("lucky_spin_save.txt");
    check(resolved.find("luckyspin/lucky_spin_save.txt") != std::string::npos,
          "resolve() places the save in the data directory");

    check(!savepath::fileExists("/tmp/luckyspin-test-home/definitely-not-here"),
          "fileExists is false for a missing file");
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    std::cout << "\033[1mLucky Spin Casino - core unit tests\033[0m\n";

    testTextWidth();
    testMoney();
    testRng();
    testDeck();
    testSavePath();

    std::cout << "\n" << std::string(60, '-') << "\n";
    if (g_failures == 0) {
        std::cout << "\033[1;32mAll " << g_checks << " checks passed.\033[0m\n";
        return 0;
    }
    std::cout << "\033[1;31m" << g_failures << " of " << g_checks
              << " checks FAILED.\033[0m\n";
    return 1;
}
