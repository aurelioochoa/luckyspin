/*
 * money.h - Currency formatting for large incremental-game numbers
 *
 * Idle game balances grow exponentially, so a fixed set of suffixes runs out
 * fast. The previous implementation stopped at "T" and rendered 1e18 as
 * "1000000.00T". This scales through the standard short-scale suffixes and
 * falls back to scientific notation beyond them.
 *
 * Pure and dependency-free so it can be unit tested.
 */

#ifndef CORE_MONEY_H
#define CORE_MONEY_H

#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <cstddef>

namespace money {

// Short-scale suffixes, each 1000x the previous.
// index 0 => 1e3, 1 => 1e6, 2 => 1e9, ...
inline const char* const* suffixes(std::size_t& count) {
    static const char* const kSuffixes[] = {
        "K",  // thousand      1e3
        "M",  // million       1e6
        "B",  // billion       1e9
        "T",  // trillion      1e12
        "Qa", // quadrillion   1e15
        "Qi", // quintillion   1e18
        "Sx", // sextillion    1e21
        "Sp", // septillion    1e24
        "Oc", // octillion     1e27
        "No", // nonillion     1e30
        "Dc", // decillion     1e33
        "UDc",// undecillion   1e36
        "DDc",// duodecillion  1e39
    };
    count = sizeof(kSuffixes) / sizeof(kSuffixes[0]);
    return kSuffixes;
}

/*
 * format() - Render an amount as a compact currency string.
 *
 * Examples:
 *   0.5        -> "$0.50"
 *   1234       -> "$1.23K"
 *   4.3e9      -> "$4.33B"
 *   1e18       -> "$1.00Qi"
 *   1e42       -> "$1.00e42"
 *   -25        -> "-$25.00"
 *   NaN        -> "$0.00"
 */
inline std::string format(double amount) {
    // Guard against NaN/inf reaching the UI.
    if (!(amount == amount)) return "$0.00";           // NaN
    if (amount > 1e308 || amount < -1e308) return "$Inf";

    const bool negative = amount < 0.0;
    double value = negative ? -amount : amount;

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    if (value < 1e3) {
        oss << value;
    } else {
        std::size_t count = 0;
        const char* const* table = suffixes(count);

        // Which 1000-power bucket are we in? 1e3 => tier 1.
        int tier = static_cast<int>(std::floor(std::log10(value) / 3.0));
        if (tier < 1) tier = 1;

        if (tier > static_cast<int>(count)) {
            // Past our suffix table: use scientific notation.
            const int exponent = static_cast<int>(std::floor(std::log10(value)));
            const double mantissa = value / std::pow(10.0, exponent);
            oss << mantissa << "e" << exponent;
        } else {
            const double scaled = value / std::pow(10.0, tier * 3);
            oss << scaled << table[tier - 1];
        }
    }

    return negative ? ("-$" + oss.str()) : ("$" + oss.str());
}

} // namespace money

/*
 * formatMoney() - Backwards-compatible free function used throughout the game.
 */
inline std::string formatMoney(double amount) {
    return money::format(amount);
}

#endif // CORE_MONEY_H
