#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <iomanip>
#include <climits>
#include <vector>
#include <sstream>

#include "gambling_game.h"
#include "colors.h"
#include "symbols.h"
#include "core/money.h"
#include "core/rng.h"
#include "core/economy.h"
#include "core/plinko.h"
#include "core/roulette.h"

// ==========================================================================
// HEADLESS GAME ACTIONS (shared by the terminal and web frontends)
// ==========================================================================
/*
 * Every rule below was extracted verbatim from the interactive loops in
 * game_games.cpp / game_shops.cpp so both frontends execute identical math.
 *
 * These methods are deliberately quiet: they mutate state and return result
 * structs for the callers to render. Two kinds of announcement still print
 * from shared code - achievements, milestones and random-event banners -
 * because those fire from deep inside economy updates; both frontends also
 * receive them through the notification queue (see takeNotifications()).
 */

/*
 * placeBet() - Common wager guard: affordable, then deducted.
 */
static bool placeBet(GamblingGame& self, double& money, double bet) {
    (void)self;
    if (bet <= 0.0 || !(bet == bet) /* NaN */ || money < bet) return false;
    money -= bet;
    return true;
}

// ------------------------------------------------------------------------
// WHEEL
// ------------------------------------------------------------------------

WheelRound GamblingGame::spinWheelRound(double bet) {
    WheelRound round{};
    if (!placeBet(*this, money, bet)) return round;

    round.played = true;
    round.bet = bet;

    const double winChance = getUpgradeValue(UpgradeType::WHEEL_CHANCE);
    const double payoutMult = getUpgradeValue(UpgradeType::WHEEL_PAYOUT);

    // chancePercent() honours fractional percentages exactly.
    round.won = Rng::global().chancePercent(winChance);

    if (round.won) {
        double winAmount = bet * payoutMult;

        const double eventMult = economy::eventPayoutMultiplier(activeEventId);
        if (eventMult != 1.0) {
            round.hotStreak = true;
            winAmount *= eventMult;
        }

        const double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
        if (Rng::global().chancePercent(luckyChance)) {
            round.luckyBonus = true;
            winAmount *= 2.0;
        }

        money += winAmount;
        round.payout = winAmount;
        recordWin(winAmount);
    } else {
        recordLoss();
    }

    incrementRound();
    return round;
}

// ------------------------------------------------------------------------
// SLOTS
// ------------------------------------------------------------------------

SlotsRound GamblingGame::playSlotsRound(double bet) {
    SlotsRound round{};
    if (!placeBet(*this, money, bet)) return round;

    round.played = true;
    round.bet = bet;

    const double winChance = getUpgradeValue(UpgradeType::SLOTS_CHANCE);
    const double payoutMult = getUpgradeValue(UpgradeType::SLOTS_PAYOUT);

    // Final reel positions: random, then forced to match when the upgraded
    // win roll succeeds. A natural triple can still land without the roll,
    // exactly like the terminal animation has always allowed.
    for (int i = 0; i < 3; i++) {
        round.reels[i] = Rng::global().below(Symbols::COUNT);
    }
    if (Rng::global().chancePercent(winChance)) {
        round.reels[1] = round.reels[0];
        round.reels[2] = round.reels[0];
    }

    const bool triple = round.reels[0] == round.reels[1]
                     && round.reels[1] == round.reels[2];
    round.won = triple;

    if (triple) {
        double winAmount = bet * payoutMult;

        const double eventMult = economy::eventPayoutMultiplier(activeEventId);
        if (eventMult != 1.0) {
            round.hotStreak = true;
            winAmount *= eventMult;
        }

        const double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
        if (Rng::global().chancePercent(luckyChance)) {
            round.luckyBonus = true;
            winAmount *= 2.0;
        }

        money += winAmount;
        round.payout = winAmount;
        recordWin(winAmount);
    } else {
        recordLoss();
    }

    incrementRound();
    return round;
}

// ------------------------------------------------------------------------
// ROULETTE
// ------------------------------------------------------------------------

RouletteRound GamblingGame::playRouletteRound(double bet, int betType,
                                              int chosenNumber) {
    RouletteRound round{};
    if (betType < 1 || betType > 5) return round;          // cancel/garbage
    if (betType == 5 && (chosenNumber < 0 || chosenNumber >= roulette::POCKETS)) {
        return round;
    }
    if (!placeBet(*this, money, bet)) return round;

    round.played = true;
    round.bet = bet;
    round.betType = betType;
    round.chosenNumber = chosenNumber;

    const roulette::BetType type = (roulette::BetType)betType;
    const double payoutUpgrade = getUpgradeValue(UpgradeType::ROULETTE_PAYOUT);

    // below() is bias-free across all 37 pockets.
    round.result = Rng::global().below(roulette::POCKETS);
    round.won = roulette::wins(type, round.result, chosenNumber);

    if (round.won) {
        double winAmount = bet * roulette::payoutMultiplier(type, payoutUpgrade);

        // Lucky 7s upgrade: flat bonus on top before any multipliers stack.
        round.luckySeven = roulette::isLuckyNumber(round.result)
                        && getUpgradeValue(UpgradeType::LUCKY_NUMBER) > 0.0;
        if (round.luckySeven) {
            winAmount += bet * getUpgradeValue(UpgradeType::LUCKY_NUMBER);
        }

        const double eventMult = economy::eventPayoutMultiplier(activeEventId);
        if (eventMult != 1.0) {
            round.hotStreak = true;
            winAmount *= eventMult;
        }

        const double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
        if (Rng::global().chancePercent(luckyChance)) {
            round.luckyBonus = true;
            winAmount *= 2.0;
        }

        money += winAmount;
        round.payout = winAmount;
        recordWin(winAmount);
    } else {
        recordLoss();
    }

    incrementRound();
    return round;
}

// ------------------------------------------------------------------------
// PLINKO
// ------------------------------------------------------------------------

PlinkoRound GamblingGame::dropPlinkoBall(double bet, int risk) {
    PlinkoRound round{};
    if (risk < 0 || risk >= plinko::RISK_COUNT) return round;
    if (!placeBet(*this, money, bet)) return round;

    round.played = true;
    round.bet = bet;
    round.risk = risk;

    // Precompute the ball path: path[r] is the horizontal index (0..r) after
    // r rows, each row a fair left/right bounce (binomial walk). The web GUI
    // animates along this exact path.
    round.path.assign(plinko::ROWS + 1, 0);
    for (int r = 1; r <= plinko::ROWS; ++r) {
        round.path[r] = round.path[r - 1] + Rng::global().below(2);
    }
    round.bucket = round.path[plinko::ROWS];

    const plinko::Risk level = (plinko::Risk)risk;
    double gross = plinko::payout(bet, level, round.bucket,
                                  getUpgradeValue(UpgradeType::PLINKO_PAYOUT),
                                  getUpgradeValue(UpgradeType::PLINKO_EDGE));

    // Specialization 5 (Plinko Pro): +50% to profitable drops.
    if (specializationId == 5 && gross > 0) {
        gross *= getSpecializationMultiplier();
    }

    const bool edge = plinko::isEdgeBucket(round.bucket);
    round.jackpot = edge;

    if (gross > 0) {
        const double eventMult = economy::eventPayoutMultiplier(activeEventId);
        if (eventMult != 1.0) {
            round.hotStreak = true;
            gross *= eventMult;
        }

        const double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
        if (Rng::global().chancePercent(luckyChance)) {
            round.luckyBonus = true;
            gross *= 2.0;
        }

        money += gross;
        round.payout = gross;
    }

    // A profitable (or break-even) drop counts as a win for stats/streaks.
    if (gross >= bet && gross > 0) {
        recordWin(gross);
    } else {
        recordLoss();
    }

    // Jackpot achievement lives with the rules so both frontends award it.
    if (edge && !achPlinkoJackpot) {
        achPlinkoJackpot = true;
        showAchievement("Plinko Jackpot", "Land in a Plinko edge bucket! (+1% income)");
        updateAchievementMultiplier();
    }

    incrementRound();
    return round;
}

// ------------------------------------------------------------------------
// BLACKJACK STATE MACHINE
// ------------------------------------------------------------------------

bool GamblingGame::blackjackDeal(double bet) {
    if (bjPhase == BlackjackPhase::PlayerTurn) return false; // hand live
    if (bet <= 0.0 || money < bet) return false;

    if (shoe.needsShuffle()) {
        shoe.shuffle(Rng::global());
    }

    money -= bet;
    bjBet = bet;
    bjPlayer.clear();
    bjDealer.clear();
    bjSettled = BlackjackSettlement{};

    // Player, dealer, player, dealer - exactly the terminal deal order.
    bjPlayer.push_back(shoe.draw(Rng::global()));
    bjDealer.push_back(shoe.draw(Rng::global()));
    bjPlayer.push_back(shoe.draw(Rng::global()));
    bjDealer.push_back(shoe.draw(Rng::global()));

    bjPhase = BlackjackPhase::PlayerTurn;
    return true;
}

BlackjackPhase GamblingGame::blackjackPhase() const {
    return bjPhase;
}

/*
 * blackjackHit() - Draw one card.
 *
 * Drawing past 21 settles the hand immediately as a bust. Hitting at 21 is
 * ignored (nothing to improve), matching the old UI which never offered it.
 * Returns the settlement when the hand ended; an empty settlement (outcome
 * None) means the player turn continues.
 */
BlackjackSettlement GamblingGame::blackjackStandFromHit(int newTotal) {
    if (newTotal > 21) {
        bjSettled = BlackjackSettlement{};
        bjSettled.outcome = BlackjackOutcome::PlayerBust;
        bjSettled.bet = bjBet;
        bjSettled.payout = 0.0;
        recordLoss();
        incrementRound();
        bjPhase = BlackjackPhase::Settled;
    }
    return bjSettled;
}

BlackjackSettlement GamblingGame::blackjackHit() {
    if (bjPhase != BlackjackPhase::PlayerTurn) return BlackjackSettlement{};
    if (cards::handTotal(bjPlayer) >= 21) return BlackjackSettlement{};

    bjPlayer.push_back(shoe.draw(Rng::global()));
    return blackjackStandFromHit(cards::handTotal(bjPlayer));
}

/*
 * blackjackStand() - Finish the player's hand and play out the dealer.
 *
 * Dealer draws until 17+, then the Dealer Jinx upgrade may force one extra
 * card (only if the dealer did not already bust) - the same sequence the
 * terminal has always shown.
 */
BlackjackSettlement GamblingGame::blackjackStand() {
    if (bjPhase != BlackjackPhase::PlayerTurn) return BlackjackSettlement{};

    const int playerTotal = cards::handTotal(bjPlayer);
    if (playerTotal > 21) {
        // Defensive: a caller skipped the bust handling in hit().
        return blackjackStandFromHit(playerTotal);
    }

    bjSettled = BlackjackSettlement{};
    bjSettled.bet = bjBet;
    bjSettled.playerNatural = cards::isNaturalBlackjack(bjPlayer);

    int dealerTotal = cards::handTotal(bjDealer);

    const double dealerBustChance = getUpgradeValue(UpgradeType::DEALER_BUST);
    const bool forcedBust = Rng::global().chancePercent(dealerBustChance);

    while (dealerTotal < 17) {
        const cards::Card drawn = shoe.draw(Rng::global());
        bjDealer.push_back(drawn);
        bjSettled.dealerDraws.push_back(drawn);
        dealerTotal = cards::handTotal(bjDealer);
    }

    if (forcedBust && dealerTotal <= 21) {
        const cards::Card bustCard = shoe.draw(Rng::global());
        bjDealer.push_back(bustCard);
        bjSettled.dealerDraws.push_back(bustCard);
        dealerTotal = cards::handTotal(bjDealer);
        bjSettled.jinx = true;
    }

    const bool dealerBust = dealerTotal > 21;
    bjSettled.dealerBust = dealerBust;
    const bool push = !dealerBust && playerTotal == dealerTotal;
    const bool playerWins = dealerBust || playerTotal > dealerTotal;

    if (push) {
        bjSettled.outcome = BlackjackOutcome::Push;
        bjSettled.payout = bjBet;
        money += bjBet;
    } else if (playerWins) {
        double basePayout = getUpgradeValue(UpgradeType::BLACKJACK_PAYOUT);
        if (bjSettled.playerNatural) basePayout += 0.5;

        double winAmount = bjBet * basePayout;

        const double eventMult = economy::eventPayoutMultiplier(activeEventId);
        if (eventMult != 1.0) {
            bjSettled.hotStreak = true;
            winAmount *= eventMult;
        }

        const double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
        if (Rng::global().chancePercent(luckyChance)) {
            bjSettled.luckyBonus = true;
            winAmount *= 2.0;
        }

        money += winAmount;
        bjSettled.outcome = BlackjackOutcome::Win;
        bjSettled.payout = winAmount;
        recordWin(winAmount);
    } else {
        bjSettled.outcome = BlackjackOutcome::Loss;
        recordLoss();
    }

    incrementRound();
    bjPhase = BlackjackPhase::Settled;
    return bjSettled;
}

/*
 * blackjackForfeit() - Walk away from a live hand; the bet is lost.
 */
BlackjackSettlement GamblingGame::blackjackForfeit() {
    if (bjPhase != BlackjackPhase::PlayerTurn) return BlackjackSettlement{};

    bjSettled = BlackjackSettlement{};
    bjSettled.bet = bjBet;
    bjSettled.outcome = BlackjackOutcome::Forfeited;
    recordLoss();
    incrementRound();
    bjPhase = BlackjackPhase::Settled;
    return bjSettled;
}

const vector<cards::Card>& GamblingGame::bjPlayerCards() const { return bjPlayer; }
const vector<cards::Card>& GamblingGame::bjDealerCards() const { return bjDealer; }
double GamblingGame::bjHandBet() const { return bjBet; }
BlackjackOutcome GamblingGame::bjLastOutcome() const { return bjSettled.outcome; }
const BlackjackSettlement& GamblingGame::lastSettlement() const { return bjSettled; }

// ------------------------------------------------------------------------
// SHOPS / BET / SPECIALIZATION / PRESTIGE CORE
// ------------------------------------------------------------------------

bool GamblingGame::buyGeneratorCount(int index, int amount,
                                     double& costOut, int& boughtOut) {
    costOut = 0.0;
    boughtOut = 0;
    if (index < 0 || index >= (int)generators.size()) return false;

    Generator& gen = generators[index];
    int count = (amount < 0) ? 1 : amount;

    if (count == 0) {
        // MAX mode: buy as many as the balance allows.
        count = getGeneratorMaxAffordable(gen, money);
        if (count <= 0) {
            costOut = gen.getCurrentCost();
            return false;
        }
    }

    if (count > INT_MAX - gen.getOwned()) {
        count = INT_MAX - gen.getOwned();
    }
    if (count <= 0) return false;

    const double cost = getGeneratorBulkCost(gen, count);
    costOut = cost;
    if (money < cost) return false;

    money -= cost;
    gen.setOwned(gen.getOwned() + count);
    boughtOut = count;

    // Buying can cross achievement thresholds (e.g. Generator Tycoon).
    checkAchievements();
    return true;
}

bool GamblingGame::buyUpgradeLevel(int index, double& costOut) {
    costOut = 0.0;
    if (index < 0 || index >= (int)upgrades.size()) return false;

    Upgrade& upg = upgrades[index];
    if (upg.isMaxed()) return false;

    const double cost = upg.getCurrentCost();
    costOut = cost;
    if (money < cost) return false;

    money -= cost;
    upg.purchase();

    // Maxing everything is itself an achievement condition.
    checkAchievements();
    return true;
}

bool GamblingGame::buyPerkLevel(int perkIndex, int& costOut) {
    static const int PERK_MAX[4] = {10, 10, 5, 20};
    static const int PERK_BASE_COST[4] = {3, 5, 8, 4};

    int level = 0;
    switch (perkIndex) {
        case 0: level = perkStartingMoney; break;
        case 1: level = perkGeneratorDiscount; break;
        case 2: level = perkOfflineCap; break;
        case 3: level = perkPassiveBoost; break;
        default: return false;
    }

    if (level >= PERK_MAX[perkIndex]) return false;

    const int cost = PERK_BASE_COST[perkIndex] + level * 2;
    costOut = cost;
    if (prestigeStars < cost) return false;

    prestigeStars -= cost;
    switch (perkIndex) {
        case 0: perkStartingMoney++; break;
        case 1: perkGeneratorDiscount++; break;
        case 2: perkOfflineCap++; break;
        case 3: perkPassiveBoost++; break;
    }
    return true;
}

bool GamblingGame::setBetAmount(double amount) {
    if (amount <= 0.0 || !(amount == amount) || amount > money) return false;
    betAmount = amount;
    return true;
}

void GamblingGame::setSpecializationId(int id) {
    if (id < 0 || id > 5) return;
    specializationId = id;
}

/*
 * executePrestige() - The full prestige reset, minus confirmation and
 * rendering. Returns stars earned so callers can celebrate appropriately.
 */
int GamblingGame::executePrestige() {
    const int starsEarned = calculatePrestigeStars();
    prestigeStars += starsEarned;

    prestigeLevel++;
    prestigeMultiplier += 0.1;
    totalLifetimeEarnings += lifetimeEarnings;

    // Reset progress (starting-money perk decides the new balance).
    money = getStartingMoneyFromPerk();
    betAmount = 10.0;
    round = 1;
    totalWins = 0;
    totalLosses = 0;
    peakMoney = money;
    lifetimeEarnings = 0.0;
    currentWinStreak = 0;

    for (auto& gen : generators) {
        gen.setOwned(0);
    }
    for (auto& upg : upgrades) {
        upg.setLevel(0);
    }

    // A fresh balance means fresh income timing.
    lastUpdateTime = time(0);

    checkPrestigeAchievements();
    checkMilestones();
    return starsEarned;
}

// ------------------------------------------------------------------------
// SNAPSHOT GETTERS
// ------------------------------------------------------------------------

double GamblingGame::getMoney() const { return money; }
double GamblingGame::getCurrentBet() const { return betAmount; }
int GamblingGame::getRound() const { return round; }
int GamblingGame::getWins() const { return totalWins; }
int GamblingGame::getLosses() const { return totalLosses; }
double GamblingGame::getPeakMoney() const { return peakMoney; }
double GamblingGame::getLifetimeEarnings() const { return lifetimeEarnings; }
int GamblingGame::getPrestigeLevel() const { return prestigeLevel; }
double GamblingGame::getPrestigeMultiplier() const { return prestigeMultiplier; }
double GamblingGame::getTotalLifetimeEarnings() const { return totalLifetimeEarnings; }
int GamblingGame::getPrestigeStars() const { return prestigeStars; }
const vector<Generator>& GamblingGame::allGenerators() const { return generators; }
const vector<Upgrade>& GamblingGame::allUpgrades() const { return upgrades; }
int GamblingGame::getSpecializationId() const { return specializationId; }
bool GamblingGame::isInterestUnlocked() const { return interestUnlocked; }
double GamblingGame::getInterestRate() const { return interestRate; }
int GamblingGame::getActiveEventId() const { return activeEventId; }
time_t GamblingGame::getActiveEventEndTime() const { return eventEndTime; }
time_t GamblingGame::getNextEventTime() const { return nextEventTime; }

vector<string> GamblingGame::takeNotifications() {
    vector<string> drained;
    drained.swap(notificationQueue);
    return drained;
}

void GamblingGame::incrementRound() { round++; }
