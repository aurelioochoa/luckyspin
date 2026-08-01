/*
 * generator.h - Passive income sources
 *
 * Generator - Represents a passive income source in the incremental game
 *
 * This is the core mechanic of incremental/idle games. Generators produce
 * money automatically over time, even when the player isn't actively playing.
 *
 * Moved out of gambling_game.cpp so that economy calculations operating on
 * vector<Generator> can be unit tested without pulling in the terminal UI.
 */

#ifndef CORE_GENERATOR_H
#define CORE_GENERATOR_H

#include <string>
#include <cmath>

class Generator {
private:
    std::string name;        // Display name of the generator
    std::string shortName;   // Short 2-3 letter code for compact display
    std::string description; // Flavor text describing the generator
    std::string icon;        // Unicode icon for visual display
    double baseIncome;       // Base income per second when you own 1
    double baseCost;         // Cost to buy the first one
    double costMultiplier;   // Each purchase multiplies cost by this amount
    int owned;               // How many of this generator the player owns

public:
    /*
     * Constructor with initialization list
     *
     * All parameters are passed to initialize the generator's properties.
     * The 'owned' count starts at 0 - player must purchase generators.
     */
    Generator(const std::string& n, const std::string& sn,
              const std::string& desc, const std::string& ico,
              double income, double cost, double mult)
        : name(n), shortName(sn), description(desc), icon(ico),
          baseIncome(income), baseCost(cost), costMultiplier(mult), owned(0) {}

    // ========================================================================
    // GETTER METHODS (Accessors)
    // ========================================================================

    std::string getName() const { return name; }
    std::string getShortName() const { return shortName; }
    std::string getDescription() const { return description; }
    std::string getIcon() const { return icon; }
    int getOwned() const { return owned; }
    double getBaseIncome() const { return baseIncome; }
    double getBaseCost() const { return baseCost; }
    double getCostMultiplier() const { return costMultiplier; }

    /*
     * getCurrentCost() - Calculate the cost to buy the next generator
     *
     * Standard incremental game cost formula:
     *   cost = baseCost * (costMultiplier ^ owned)
     *
     * Example with baseCost=10, costMultiplier=1.15:
     *   - 1st purchase: 10 * 1.15^0 = 10.00
     *   - 2nd purchase: 10 * 1.15^1 = 11.50
     *   - 3rd purchase: 10 * 1.15^2 = 13.23
     */
    double getCurrentCost() const {
        return baseCost * std::pow(costMultiplier, owned);
    }

    /*
     * getCostAtOwned() - Cost of the next unit if `count` were already owned.
     *
     * Used by bulk-purchase pricing, which needs the cost of units the player
     * does not own yet.
     */
    double getCostAtOwned(int count) const {
        return baseCost * std::pow(costMultiplier, count);
    }

    /*
     * getIncomePerSecond() - Calculate total income from all owned generators
     */
    double getIncomePerSecond() const {
        return baseIncome * owned;
    }

    // ========================================================================
    // MODIFIER METHODS
    // ========================================================================

    /*
     * purchase() - Buy one more of this generator
     *
     * The actual money deduction is handled by the game class to keep
     * responsibilities separate.
     */
    void purchase() {
        if (owned < MAX_OWNED) owned++;
    }

    /*
     * setOwned() - Set the owned count directly (for loading saved games)
     */
    void setOwned(int count) {
        // Clamp: negative counts would produce negative income, and absurdly
        // large counts overflow pow() in getCurrentCost() to infinity.
        if (count < 0) count = 0;
        if (count > MAX_OWNED) count = MAX_OWNED;
        owned = count;
    }

    static const int MAX_OWNED = 100000;

    /*
     * reset() - Reset generator to initial state (for game restart)
     */
    void reset() {
        owned = 0;
    }
};

#endif // CORE_GENERATOR_H
