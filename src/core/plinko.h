/*
 * plinko.h - Headless Plinko board math
 *
 * A Plinko ball falls through ROWS rows of pegs; at each peg it bounces
 * left or right with equal probability, so the final bucket follows a
 * binomial distribution: P(bucket k) = C(ROWS, k) / 2^ROWS.
 *
 * Everything here is pure (no terminal, no member state) so the payout
 * tables and their expected values can be unit tested. The house edge
 * lives entirely in the multiplier tables below.
 */

#ifndef CORE_PLINKO_H
#define CORE_PLINKO_H

#include <cmath>

#include "rng.h"

namespace plinko {

// Board geometry: 10 rows of pegs, 11 buckets.
const int ROWS = 10;
const int BUCKETS = ROWS + 1;

/*
 * Risk - Player-chosen volatility.
 *
 * LOW:    gentle table, center buckets return ~most of the bet.
 * MEDIUM: wider spread, moderate edge jackpots.
 * HIGH:   most buckets lose, edge buckets pay a real jackpot.
 */
enum class Risk { LOW = 0, MEDIUM = 1, HIGH = 2 };
const int RISK_COUNT = 3;

/*
 * Multiplier tables, one row per risk level, indexed by bucket 0..ROWS.
 * Buckets are symmetric (bucket k and ROWS-k pay the same) because the
 * distribution is symmetric.
 *
 * Expected values (see expectedValue(), asserted in tests):
 *   LOW ~0.967   MEDIUM ~0.936   HIGH ~0.871
 */
inline const double* multiplierTable(Risk risk) {
    static const double LOW_TABLE[BUCKETS] =
        { 2.0, 1.5, 1.2, 1.1, 1.0, 0.65, 1.0, 1.1, 1.2, 1.5, 2.0 };
    static const double MEDIUM_TABLE[BUCKETS] =
        { 6.0, 3.0, 1.8, 1.2, 0.8, 0.4, 0.8, 1.2, 1.8, 3.0, 6.0 };
    static const double HIGH_TABLE[BUCKETS] =
        { 35.0, 9.0, 2.5, 1.0, 0.3, 0.2, 0.3, 1.0, 2.5, 9.0, 35.0 };

    switch (risk) {
        case Risk::LOW:    return LOW_TABLE;
        case Risk::MEDIUM: return MEDIUM_TABLE;
        case Risk::HIGH:   return HIGH_TABLE;
    }
    return LOW_TABLE;
}

/*
 * binomialCoefficient() - C(n, k) computed iteratively to avoid overflow
 * from naive factorials. Exact for the small n used here.
 */
inline double binomialCoefficient(int n, int k) {
    if (k < 0 || k > n) return 0.0;
    if (k > n - k) k = n - k;
    double result = 1.0;
    for (int i = 0; i < k; ++i) {
        result *= (double)(n - i);
        result /= (double)(i + 1);
    }
    return result;
}

/*
 * bucketProbability() - P(a fair ball lands in bucket k).
 */
inline double bucketProbability(int bucket) {
    if (bucket < 0 || bucket > ROWS) return 0.0;
    return binomialCoefficient(ROWS, bucket) / std::pow(2.0, ROWS);
}

/*
 * expectedValue() - Mean multiplier returned per 1.0 bet on a risk level.
 *
 * Sum over buckets of probability * multiplier. Below 1.0 the house wins
 * on average; the distance from 1.0 is the house edge.
 */
inline double expectedValue(Risk risk) {
    const double* table = multiplierTable(risk);
    double ev = 0.0;
    for (int k = 0; k < BUCKETS; ++k) {
        ev += bucketProbability(k) * table[k];
    }
    return ev;
}

/*
 * dropBall() - Simulate one fair ball drop.
 *
 * Returns the final bucket index in [0, ROWS]: the count of right bounces
 * out of ROWS pegs, each a fair coin flip.
 */
inline int dropBall(Rng& rng) {
    int bucket = 0;
    for (int row = 0; row < ROWS; ++row) {
        if (rng.below(2) == 1) ++bucket;
    }
    return bucket;
}

/*
 * isEdgeBucket() - The two outermost buckets hold the jackpot multipliers.
 */
inline bool isEdgeBucket(int bucket) {
    return bucket == 0 || bucket == ROWS;
}

/*
 * payout() - Total amount returned to the player for one drop.
 *
 *   bet * table[bucket] * payoutUpgrade * (edgeUpgrade on edge buckets)
 *
 * payoutUpgrade scales every bucket (PLINKO_PAYOUT, base 1.0).
 * edgeUpgrade scales only the jackpot buckets (PLINKO_EDGE, base 1.0).
 * Both are upgrade values from Upgrade::getCurrentValue(), so they compose
 * with the shop UI without special cases here.
 */
inline double payout(double bet, Risk risk, int bucket,
                     double payoutUpgrade, double edgeUpgrade) {
    if (bucket < 0 || bucket > ROWS) return 0.0;
    const double* table = multiplierTable(risk);
    double mult = table[bucket] * payoutUpgrade;
    if (isEdgeBucket(bucket)) {
        mult *= edgeUpgrade;
    }
    return bet * mult;
}

} // namespace plinko

#endif // CORE_PLINKO_H
