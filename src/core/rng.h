/*
 * rng.h - Random number generation for Lucky Spin Casino
 *
 * Replaces rand()/srand(). Reasons this matters here specifically:
 *
 *   1. Determinism for tests. A seeded Rng lets a test assert that a 25%
 *      chance really fires ~25% of the time; rand() with a global hidden
 *      seed cannot be tested.
 *   2. Fractional probabilities. `rand() % 100 < 62.5` actually pays out 63%
 *      of the time, because the left side is an integer. chancePercent()
 *      compares against a real number in [0, 100).
 *   3. No modulo bias. below() uses a uniform distribution rather than `%`,
 *      which skews slightly when the range does not divide RAND_MAX evenly.
 *
 * Pure and dependency-free so it can be unit tested.
 */

#ifndef CORE_RNG_H
#define CORE_RNG_H

#include <random>
#include <cstdint>
#include <ctime>
#include <algorithm>

class Rng {
public:
    // Deterministic construction - used by tests.
    explicit Rng(std::uint32_t seed) : engine_(seed) {}

    // Non-deterministic construction - used by the game.
    Rng() : engine_(nonDeterministicSeed()) {}

    void reseed(std::uint32_t seed) { engine_.seed(seed); }

    /*
     * below() - Uniform integer in [0, n). Returns 0 when n <= 0.
     * Bias-free, unlike `rand() % n`.
     */
    int below(int n) {
        if (n <= 1) return 0;
        std::uniform_int_distribution<int> dist(0, n - 1);
        return dist(engine_);
    }

    /*
     * range() - Uniform integer in [lo, hi], inclusive on both ends.
     */
    int range(int lo, int hi) {
        if (lo > hi) std::swap(lo, hi);
        std::uniform_int_distribution<int> dist(lo, hi);
        return dist(engine_);
    }

    /*
     * unit() - Uniform real in [0.0, 1.0).
     */
    double unit() {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        return dist(engine_);
    }

    /*
     * chancePercent() - True with the given percentage probability.
     *
     * Accepts fractional percentages exactly: chancePercent(62.5) fires
     * 62.5% of the time, not 63%.
     */
    bool chancePercent(double percent) {
        if (percent <= 0.0) return false;
        if (percent >= 100.0) return true;
        return unit() * 100.0 < percent;
    }

    /*
     * global() - The game's shared generator.
     *
     * A single instance keeps every draw coming from one well-distributed
     * stream. Tests construct their own Rng instead of touching this.
     */
    static Rng& global() {
        static Rng instance;
        return instance;
    }

private:
    static std::uint32_t nonDeterministicSeed() {
        std::random_device rd;
        // random_device may be a stub on some platforms; mix in the clock so a
        // constant rd() still yields a different stream per run.
        return static_cast<std::uint32_t>(rd()) ^
               static_cast<std::uint32_t>(std::time(0));
    }

    std::mt19937 engine_;
};

#endif // CORE_RNG_H
