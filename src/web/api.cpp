#include <string>
#include <vector>
#include <sstream>
#include <ctime>
#include <cmath>

#include "api.h"
#include "json.h"
#include "web_assets.h"

#include "../colors.h"
#include "../symbols.h"
#include "../core/economy.h"
#include "../core/money.h"
#include "../core/plinko.h"
#include "../core/roulette.h"
#include "../core/deck.h"
#include "../core/generator.h"
#include "../core/upgrade.h"

namespace api {

using web::Request;
using web::Response;

// ==========================================================================
// HELPERS
// ==========================================================================

namespace {

std::string moneyStr(double v) { return json::num(v, 2); }

std::string cardJson(const cards::Card& card) {
    std::ostringstream o;
    o << "{\"rank\":" << json::integer(card.rank)
      << ",\"label\":\"" << json::escape(cards::rankName(card.rank)) << "\""
      << ",\"suit\":\"" << json::escape(Cards::SUITS[card.suit]) << "\""
      << ",\"red\":" << json::boolean(cards::isRedSuit(card.suit)) << "}";
    return o.str();
}

std::string cardsJson(const std::vector<cards::Card>& hand) {
    std::string out = "[";
    for (size_t i = 0; i < hand.size(); i++) {
        if (i) out += ",";
        out += cardJson(hand[i]);
    }
    return out + "]";
}

/*
 * outcomeString() - Settlement as a client-friendly token; "null" while the
 * player is still drawing.
 */
const char* outcomeString(BlackjackOutcome o) {
    switch (o) {
        case BlackjackOutcome::Win:       return "win";
        case BlackjackOutcome::Loss:      return "loss";
        case BlackjackOutcome::Push:      return "push";
        case BlackjackOutcome::PlayerBust: return "bust";
        case BlackjackOutcome::Forfeited: return "forfeit";
        default:                          return "null";
    }
}

std::string blackjackJson(GamblingGame& game) {
    std::ostringstream o;
    const BlackjackPhase phase = game.blackjackPhase();
    const char* phaseStr =
        phase == BlackjackPhase::PlayerTurn ? "player" :
        phase == BlackjackPhase::Settled ? "settled" : "idle";

    o << "{\"phase\":\"" << phaseStr << "\",\"bet\":" << moneyStr(game.bjHandBet());

    // Player hand is always visible.
    o << ",\"player\":" << cardsJson(game.bjPlayerCards());
    if (!game.bjPlayerCards().empty()) {
        o << ",\"playerTotal\":" << cards::handTotal(game.bjPlayerCards())
          << ",\"soft\":" << json::boolean(cards::isSoft(game.bjPlayerCards()));
    }

    // Dealer hole card stays hidden during the player's turn.
    const std::vector<cards::Card>& dealer = game.bjDealerCards();
    o << ",\"dealerCards\":[";
    if (!dealer.empty()) {
        o << cardJson(dealer[0]);
        if (phase != BlackjackPhase::PlayerTurn) {
            for (size_t i = 1; i < dealer.size(); i++) {
                o << "," << cardJson(dealer[i]);
            }
        }
    }
    o << "],\"dealerHidden\":" << json::boolean(phase == BlackjackPhase::PlayerTurn)
      << ",\"dealerTotal\":";
    o << ((phase == BlackjackPhase::PlayerTurn || dealer.empty())
              ? -1
              : cards::handTotal(dealer));

    if (phase == BlackjackPhase::Settled) {
        const BlackjackSettlement& s = game.lastSettlement();
        o << ",\"outcome\":\"" << outcomeString(s.outcome) << "\""
          << ",\"payout\":" << moneyStr(s.payout)
          << ",\"betAtStake\":" << moneyStr(s.bet)
          << ",\"dealerBust\":" << json::boolean(s.dealerBust)
          << ",\"natural\":" << json::boolean(s.playerNatural)
          << ",\"jinx\":" << json::boolean(s.jinx)
          << ",\"hotStreak\":" << json::boolean(s.hotStreak)
          << ",\"luckyBonus\":" << json::boolean(s.luckyBonus);
    } else {
        o << ",\"outcome\":\"null\"";
    }
    o << "}";
    return o.str();
}

} // namespace

// ==========================================================================
// STATE SNAPSHOT
// ==========================================================================

std::string stateJson(GamblingGame& game) {
    // Polling alone keeps the idle economy alive.
    game.updatePassiveIncome();

    std::ostringstream o;

    o << "{";
    o << "\"serverTime\":" << json::integer((long long)time(0)) << ",";
    o << "\"money\":" << moneyStr(game.getMoney()) << ","
      << "\"bet\":" << moneyStr(game.getCurrentBet()) << ","
      << "\"round\":" << json::integer(game.getRound()) << ","
      << "\"wins\":" << json::integer(game.getWins()) << ","
      << "\"losses\":" << json::integer(game.getLosses()) << ","
      << "\"peak\":" << moneyStr(game.getPeakMoney()) << ","
      << "\"lifetime\":" << moneyStr(game.getLifetimeEarnings()) << ","
      << "\"passiveIncome\":" << moneyStr(game.getTotalPassiveIncome()) << ","
      << "\"prestige\":{\"level\":" << json::integer(game.getPrestigeLevel())
      << ",\"multiplier\":" << json::num(game.getPrestigeMultiplier(), 1)
      << ",\"stars\":" << json::integer(game.getPrestigeStars())
      << ",\"totalLifetime\":" << moneyStr(game.getTotalLifetimeEarnings())
      << ",\"requirement\":" << moneyStr(game.getPrestigeRequirement())
      << ",\"canPrestige\":" << json::boolean(game.canPrestige())
      << ",\"nextStars\":" << json::integer(game.calculatePrestigeStars()) << "},";
    o << "\"perks\":[";
    {
        static const char* PERK_NAMES[4]  = {"Starting Money", "Generator Discount",
                                             "Offline Cap", "Passive Boost"};
        static const char* PERK_DESCS[4]  = {"Start with more money after prestige",
                                             "All generators cost less",
                                             "Increase offline earnings cap",
                                             "Permanent passive income per level"};
        static const int   PERK_MAX[4]    = {10, 10, 5, 20};
        static const int   PERK_BASE[4]   = {3, 5, 8, 4};
        for (int p = 0; p < 4; p++) {
            if (p) o << ",";
            const int level = game.getPerkLevel(p);
            o << "{\"name\":\"" << PERK_NAMES[p]
              << "\",\"desc\":\"" << PERK_DESCS[p]
              << "\",\"level\":" << json::integer(level)
              << ",\"maxLevel\":" << PERK_MAX[p]
              << ",\"cost\":" << (PERK_BASE[p] + level * 2)
              << "}";
        }
    }
    o << "],";
    o << "\"interestUnlocked\":" << json::boolean(game.isInterestUnlocked())
      << ",\"specialization\":" << json::integer(game.getSpecializationId()) << ",";

    // Random event snapshot.
    const int eventId = game.getActiveEventId();
    o << "\"event\":{\"id\":" << eventId;
    if (eventId != economy::EVENT_NONE) {
        o << ",\"name\":\"" << economy::eventName(eventId) << "\""
          << ",\"desc\":\"" << economy::eventDescription(eventId) << "\""
          << ",\"endTime\":" << json::integer((long long)game.getActiveEventEndTime());
    }
    o << "},";

    // Slot symbols for reel rendering.
    o << "\"slotSymbols\":[";
    for (int i = 0; i < Symbols::COUNT; i++) {
        if (i) o << ",";
        o << "\"" << json::escape(Symbols::ALL[i]) << "\"";
    }
    o << "],";

    // Generators with per-mode bulk pricing.
    static const int BUY_AMOUNTS[5] = {1, 10, 25, 100, 0};   // 0 = MAX
    o << "\"generators\":[";
    const std::vector<Generator>& gens = game.allGenerators();
    for (size_t g = 0; g < gens.size(); g++) {
        const Generator& gen = gens[g];
        if (g) o << ",";
        o << "{\"name\":\"" << json::escape(gen.getName())
          << "\",\"icon\":\"" << json::escape(gen.getIcon())
          << "\",\"desc\":\"" << json::escape(gen.getDescription())
          << "\",\"owned\":" << json::integer(gen.getOwned())
          << ",\"baseIncome\":" << moneyStr(gen.getBaseIncome())
          << ",\"synergy\":" << json::num(game.getGeneratorSynergyMultiplier((int)g), 3)
          << ",\"costs\":[";
        for (int m = 0; m < 5; m++) {
            if (m) o << ",";
            if (BUY_AMOUNTS[m] == 0) {
                const int affordable = game.getGeneratorMaxAffordable(gen, game.getMoney());
                o << (affordable > 0
                          ? moneyStr(game.getGeneratorBulkCost(gen, affordable))
                          : moneyStr(gen.getCurrentCost()));
            } else {
                o << moneyStr(game.getGeneratorBulkCost(gen, BUY_AMOUNTS[m]));
            }
        }
        o << "],\"maxAffordable\":" << json::integer(game.getGeneratorMaxAffordable(gen, game.getMoney()))
          << "}";
    }
    o << "],";

    // Upgrades.
    o << "\"upgrades\":[";
    const std::vector<Upgrade>& ups = game.allUpgrades();
    for (size_t u = 0; u < ups.size(); u++) {
        const Upgrade& upg = ups[u];
        if (u) o << ",";
        o << "{\"name\":\"" << json::escape(upg.getName())
          << "\",\"desc\":\"" << json::escape(upg.getDescription())
          << "\",\"level\":" << json::integer(upg.getCurrentLevel())
          << ",\"maxLevel\":" << json::integer(upg.getMaxLevel())
          << ",\"maxed\":" << json::boolean(upg.isMaxed())
          << ",\"cost\":" << moneyStr(upg.getCurrentCost())
          << ",\"value\":" << json::num(upg.getCurrentValue(), 2)
          << ",\"nextValue\":" << json::num(upg.getNextValue(), 2)
          << "}";
    }
    o << "],";

    // Achievements (mirrors displayAchievements()).
    {
        struct Ach { const char* name; const char* desc; };
        static const Ach LIST[] = {
            {"First Win",       "Win your first game"},
            {"High Roller",     "Win $1,000 in a single bet"},
            {"Millionaire",     "Reach $1,000,000"},
            {"Lucky Streak",    "Win 10 games in a row"},
            {"Generator Tycoon","Own 10 of any generator"},
            {"Fully Upgraded",  "Max out all upgrades"},
            {"First Prestige",  "Prestige for the first time"},
            {"Prestige Master", "Reach prestige level 5"},
            {"Plinko Jackpot",  "Land in a Plinko edge bucket"},
        };
        o << "\"achievements\":[";
        for (size_t a = 0; a < sizeof(LIST)/sizeof(LIST[0]); a++) {
            if (a) o << ",";
            o << "{\"name\":\"" << LIST[a].name << "\",\"desc\":\"" << LIST[a].desc
              << "\",\"earned\":" << json::boolean(game.getAchievementEarned((int)a))
              << "}";
        }
        o << "],\"unlockedCount\":" << json::integer(game.getAchievementCount());
    }

    // Blackjack session.
    o << ",\"blackjack\":" << blackjackJson(game);

    // Drained notifications ("kind|title|desc").
    o << ",\"notifications\":[";
    {
        const std::vector<std::string> notes = game.takeNotifications();
        for (size_t n = 0; n < notes.size(); n++) {
            if (n) o << ",";
            o << "\"" << json::escape(notes[n]) << "\"";
        }
    }
    o << "]";

    o << "}";
    return o.str();
}

// ==========================================================================
// ACTION DISPATCH
// ==========================================================================

namespace {

Response jsonResponse(const std::string& body, int status = 200) {
    Response res;
    res.status = status;
    res.body = body;
    return res;
}

Response errorResponse(const std::string& message) {
    return jsonResponse("{\"ok\":false,\"message\":\"" + json::escape(message) + "\"}");
}

Response runAction(GamblingGame& game, const json::FlatObject& action) {
    const std::string name = action.getString("action");
    std::ostringstream result;

    if (name == "wheel") {
        const WheelRound r = game.spinWheelRound(action.getNumber("bet", game.getCurrentBet()));
        if (!r.played) return errorResponse("Not enough money for that bet.");
        result << "{\"round\":\"wheel\",\"won\":" << json::boolean(r.won)
               << ",\"payout\":" << moneyStr(r.payout)
               << ",\"hotStreak\":" << json::boolean(r.hotStreak)
               << ",\"luckyBonus\":" << json::boolean(r.luckyBonus) << "}";
    } else if (name == "slots") {
        const SlotsRound r = game.playSlotsRound(action.getNumber("bet", game.getCurrentBet()));
        if (!r.played) return errorResponse("Not enough money for that bet.");
        result << "{\"round\":\"slots\",\"won\":" << json::boolean(r.won)
               << ",\"reels\":[" << r.reels[0] << "," << r.reels[1] << "," << r.reels[2] << "]"
               << ",\"payout\":" << moneyStr(r.payout)
               << ",\"hotStreak\":" << json::boolean(r.hotStreak)
               << ",\"luckyBonus\":" << json::boolean(r.luckyBonus) << "}";
    } else if (name == "roulette") {
        const RouletteRound r = game.playRouletteRound(
            action.getNumber("bet", game.getCurrentBet()),
            (int)action.getNumber("type", 0),
            (int)action.getNumber("number", -1));
        if (!r.played) return errorResponse("Invalid bet or insufficient funds.");
        const char* color = r.result == 0 ? "green"
                          : roulette::isRed(r.result) ? "red" : "black";
        result << "{\"round\":\"roulette\",\"won\":" << json::boolean(r.won)
               << ",\"result\":" << r.result
               << ",\"color\":\"" << color << "\""
               << ",\"luckySeven\":" << json::boolean(r.luckySeven)
               << ",\"payout\":" << moneyStr(r.payout)
               << ",\"hotStreak\":" << json::boolean(r.hotStreak)
               << ",\"luckyBonus\":" << json::boolean(r.luckyBonus) << "}";
    } else if (name == "plinko") {
        const PlinkoRound r = game.dropPlinkoBall(
            action.getNumber("bet", game.getCurrentBet()),
            (int)action.getNumber("risk", 0));
        if (!r.played) return errorResponse("Invalid risk level or insufficient funds.");
        result << "{\"round\":\"plinko\",\"bucket\":" << r.bucket
               << ",\"jackpot\":" << json::boolean(r.jackpot)
               << ",\"path\":[";
        for (size_t i = 0; i < r.path.size(); i++) {
            if (i) result << ",";
            result << r.path[i];
        }
        result << "],\"payout\":" << moneyStr(r.payout)
               << ",\"hotStreak\":" << json::boolean(r.hotStreak)
               << ",\"luckyBonus\":" << json::boolean(r.luckyBonus) << "}";
    } else if (name == "bj_deal") {
        if (!game.blackjackDeal(action.getNumber("bet", game.getCurrentBet()))) {
            return errorResponse("Cannot deal right now (hand live or funds short).");
        }
        result << "{\"round\":\"blackjack\"}";
    } else if (name == "bj_hit") {
        if (game.blackjackPhase() != BlackjackPhase::PlayerTurn) {
            return errorResponse("No live hand to hit.");
        }
        const size_t cardsBefore = game.bjPlayerCards().size();
        const BlackjackSettlement s = game.blackjackHit();
        const bool drew = game.bjPlayerCards().size() > cardsBefore;
        result << "{\"round\":\"blackjack\",\"drew\":" << json::boolean(drew)
               << ",\"bust\":" << json::boolean(s.outcome == BlackjackOutcome::PlayerBust)
               << "}";
    } else if (name == "bj_stand") {
        const BlackjackSettlement s = game.blackjackStand();
        if (s.outcome == BlackjackOutcome::None) {
            return errorResponse("No live hand to stand on.");
        }
        result << "{\"round\":\"blackjack\",\"settled\":true,\"outcome\":\""
               << outcomeString(s.outcome) << "\",\"payout\":"
               << moneyStr(s.payout)
               << ",\"dealerBust\":" << json::boolean(s.dealerBust)
               << ",\"jinx\":" << json::boolean(s.jinx)
               << ",\"natural\":" << json::boolean(s.playerNatural)
               << ",\"hotStreak\":" << json::boolean(s.hotStreak)
               << ",\"luckyBonus\":" << json::boolean(s.luckyBonus) << "}";
    } else if (name == "buy_gen") {
        double cost = 0.0;
        int bought = 0;
        if (!game.buyGeneratorCount((int)action.getNumber("index", -1),
                                    (int)action.getNumber("amount", 1),
                                    cost, bought)) {
            return errorResponse(bought > 0 ? "Partial purchase failed."
                                            : "Cannot afford that purchase.");
        }
        result << "{\"round\":\"shop\",\"bought\":" << bought
               << ",\"cost\":" << moneyStr(cost) << "}";
    } else if (name == "buy_upgrade") {
        double cost = 0.0;
        if (!game.buyUpgradeLevel((int)action.getNumber("index", -1), cost)) {
            return errorResponse("Maxed out or unaffordable.");
        }
        result << "{\"round\":\"upgrade\",\"cost\":" << moneyStr(cost) << "}";
    } else if (name == "buy_perk") {
        int cost = 0;
        if (!game.buyPerkLevel((int)action.getNumber("index", -1), cost)) {
            return errorResponse("Maxed out or not enough stars.");
        }
        result << "{\"round\":\"perk\",\"cost\":" << cost << "}";
    } else if (name == "set_bet") {
        if (!game.setBetAmount(action.getNumber("amount", 0))) {
            return errorResponse("Bet must be positive and within your balance.");
        }
        result << "{\"round\":\"bet\"}";
    } else if (name == "set_spec") {
        game.setSpecializationId((int)action.getNumber("id", 0));
        result << "{\"round\":\"spec\"}";
    } else if (name == "prestige") {
        if (!game.canPrestige()) {
            return errorResponse("Not enough lifetime earnings to prestige yet.");
        }
        const int stars = game.executePrestige();
        result << "{\"round\":\"prestige\",\"stars\":" << stars << "}";
    } else if (name == "save") {
        if (!game.saveGame()) return errorResponse("Save failed.");
        result << "{\"round\":\"save\"}";
    } else if (name == "restart") {
        game.resetGame();
        result << "{\"round\":\"restart\"}";
    } else {
        return errorResponse("Unknown action: " + name);
    }

    // Every successful mutation is persisted immediately; saves are small,
    // atomic and infrequent enough not to matter.
    game.saveGame();
    return jsonResponse("{\"ok\":true,\"result\":" + result.str()
                        + ",\"state\":" + stateJson(game) + "}");
}

} // namespace

Response handle(GamblingGame& game, const web::Request& req) {
    if (req.method == "GET" && req.path == "/") {
        Response res;
        res.contentType = "text/html";
        res.body = assets::WEB_INDEX_HTML;
        return res;
    }
    if (req.method == "GET" && req.path == "/api/state") {
        return jsonResponse(stateJson(game));
    }
    if (req.method == "POST" && req.path == "/api/action") {
        const json::FlatParseResult parsed = json::parseFlatObject(req.body);
        if (!parsed.ok) return errorResponse("Malformed request body.");
        return runAction(game, parsed.object);
    }
    Response res;
    res.status = 404;
    res.contentType = "application/json";
    res.body = "{\"ok\":false,\"message\":\"not found\"}";
    return res;
}

} // namespace api
