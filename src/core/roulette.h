/*
 * roulette.h - Headless roulette outcome math
 *
 * European-style wheel: 37 pockets (0-36), 18 of them red. Zero loses every
 * outside bet, which is where the house edge comes from.
 *
 * Everything here is pure so the odds and payout formulas are unit-testable
 * and identical between the terminal and web frontends.
 */

#ifndef CORE_ROULETTE_H
#define CORE_ROULETTE_H

namespace roulette {

// Pockets covered by each outside bet.
const int POCKETS = 37;

/*
 * BetType - Matches the terminal menu order; 1-4 pay even money, 5 is a
 * straight-up number bet. 0/CANCEL means "no bet placed".
 */
enum class BetType { Cancel = 0, Red = 1, Black = 2, Odd = 3, Even = 4, Number = 5 };

/*
 * isRed() - Red pocket layout of a real European wheel.
 */
inline bool isRed(int n) {
    static const int redNumbers[] = {
        1, 3, 5, 7, 9, 12, 14, 16, 18, 19, 21, 23, 25, 27, 30, 32, 34, 36
    };
    if (n <= 0 || n >= POCKETS) return false;
    for (const int red : redNumbers) {
        if (red == n) return true;
    }
    return false;
}

/*
 * isLuckyNumber() - The Lucky 7s upgrade pays a bonus on these pockets.
 */
inline bool isLuckyNumber(int n) {
    return n == 7 || n == 17 || n == 27;
}

/*
 * wins() - Whether `result` settles `bet` in the player's favour.
 *
 * A spin of 0 beats Red/Black/Odd/Even but still loses to Number unless it
 * IS the chosen number.
 */
inline bool wins(BetType bet, int result, int chosen) {
    switch (bet) {
        case BetType::Red:    return result != 0 && isRed(result);
        case BetType::Black:  return result != 0 && !isRed(result);
        case BetType::Odd:    return result != 0 && result % 2 == 1;
        case BetType::Even:   return result != 0 && result % 2 == 0;
        case BetType::Number: return result == chosen;
        default:              return false;
    }
}

/*
 * payoutMultiplier() - Total multiplier applied to the bet on a win.
 *
 * Outside bets pay the ROULETTE_PAYOUT upgrade value (base 2x). A straight
 * number pays true 35x plus five times whatever the upgrade adds above its
 * base, so maxing the upgrade lifts number payouts meaningfully too.
 */
inline double payoutMultiplier(BetType bet, double basePayoutUpgrade) {
    if (bet == BetType::Number) {
        return 35.0 + (basePayoutUpgrade - 2.0) * 5.0;
    }
    return basePayoutUpgrade;
}

} // namespace roulette

#endif // CORE_ROULETTE_H
