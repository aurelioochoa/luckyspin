/*
 * deck.h - Playing cards, hand scoring, and a multi-deck shoe
 *
 * The previous blackjack implementation drew `(rand() % 13) + 1` and clamped
 * anything above 10 down to 10. Two consequences:
 *
 *   1. J, Q and K could never be displayed - a third of the deck rendered as
 *      "10", and the getCardName() branches for them were unreachable.
 *   2. There was no deck. Cards were drawn with replacement from an infinite
 *      shoe, so five aces in a row was possible and the suit shown came from
 *      the card's position in the hand rather than the card itself.
 *
 * This models a real shoe: a finite, shuffled multi-deck stack that is dealt
 * from without replacement and reshuffled when it runs low.
 *
 * Pure and dependency-free (apart from Rng) so it can be unit tested.
 */

#ifndef CORE_DECK_H
#define CORE_DECK_H

#include <string>
#include <vector>
#include <cstddef>
#include "rng.h"

namespace cards {

// Suit indices, matching Cards::SUITS in symbols.h
enum Suit { SPADES = 0, HEARTS = 1, DIAMONDS = 2, CLUBS = 3 };

struct Card {
    int rank;  // 1 = Ace, 2..10 = pip, 11 = Jack, 12 = Queen, 13 = King
    int suit;  // 0..3, see Suit

    Card() : rank(1), suit(0) {}
    Card(int r, int s) : rank(r), suit(s) {}
};

/*
 * blackjackValue() - Point value of a rank.
 *
 * Aces count as 1 here; handTotal() promotes one to 11 when it helps.
 * Face cards are worth 10 but keep their distinct rank for display.
 */
inline int blackjackValue(int rank) {
    if (rank >= 10) return 10;   // 10, J, Q, K
    return rank;                  // A counts as 1 here
}

/*
 * rankName() - Short display name for a rank.
 */
inline std::string rankName(int rank) {
    switch (rank) {
        case 1:  return "A";
        case 11: return "J";
        case 12: return "Q";
        case 13: return "K";
        default: {
            // 2..10
            std::string out;
            int n = rank;
            if (n >= 10) { out += char('0' + n / 10); n %= 10; }
            out += char('0' + n);
            return out;
        }
    }
}

/*
 * isRedSuit() - Hearts and diamonds are red; spades and clubs are not.
 */
inline bool isRedSuit(int suit) {
    return suit == HEARTS || suit == DIAMONDS;
}

/*
 * handTotal() - Best blackjack total for a hand.
 *
 * Counts every ace as 1, then promotes a single ace to 11 if doing so does not
 * bust. Only one ace can ever be promoted (two 11s would be 22).
 */
inline int handTotal(const std::vector<Card>& hand) {
    int total = 0;
    bool hasAce = false;

    for (std::size_t i = 0; i < hand.size(); ++i) {
        total += blackjackValue(hand[i].rank);
        if (hand[i].rank == 1) hasAce = true;
    }

    if (hasAce && total + 10 <= 21) {
        total += 10;   // promote one ace from 1 to 11
    }

    return total;
}

/*
 * isSoft() - True if the hand counts an ace as 11.
 */
inline bool isSoft(const std::vector<Card>& hand) {
    int hard = 0;
    bool hasAce = false;
    for (std::size_t i = 0; i < hand.size(); ++i) {
        hard += blackjackValue(hand[i].rank);
        if (hand[i].rank == 1) hasAce = true;
    }
    return hasAce && hard + 10 <= 21;
}

/*
 * isNaturalBlackjack() - 21 on exactly two cards.
 */
inline bool isNaturalBlackjack(const std::vector<Card>& hand) {
    return hand.size() == 2 && handTotal(hand) == 21;
}

/*
 * Shoe - A finite stack of shuffled decks, dealt without replacement.
 */
class Shoe {
public:
    explicit Shoe(int deckCount = 6)
        : deckCount_(deckCount < 1 ? 1 : deckCount), next_(0) {
        build();
    }

    // Total cards when full.
    std::size_t size() const { return cards_.size(); }

    // Cards left before a reshuffle is needed.
    std::size_t remaining() const { return cards_.size() - next_; }

    /*
     * shuffle() - Fisher-Yates shuffle and reset the deal position.
     *
     * Every permutation is equally likely, and each shuffle consumes only
     * `n-1` draws from the generator.
     */
    void shuffle(Rng& rng) {
        for (std::size_t i = cards_.size(); i > 1; --i) {
            const std::size_t j = static_cast<std::size_t>(rng.below(static_cast<int>(i)));
            Card tmp = cards_[i - 1];
            cards_[i - 1] = cards_[j];
            cards_[j] = tmp;
        }
        next_ = 0;
    }

    /*
     * needsShuffle() - True once penetration passes ~75% of the shoe.
     *
     * Real casinos reshuffle before the shoe is exhausted rather than dealing
     * to the last card.
     */
    bool needsShuffle() const {
        return remaining() <= cards_.size() / 4;
    }

    /*
     * draw() - Deal the next card, reshuffling first if the shoe is spent.
     */
    Card draw(Rng& rng) {
        if (next_ >= cards_.size()) {
            shuffle(rng);
        }
        return cards_[next_++];
    }

    /*
     * reset() - Rebuild a full, ordered shoe (call shuffle() afterwards).
     */
    void reset() {
        build();
    }

private:
    void build() {
        cards_.clear();
        cards_.reserve(static_cast<std::size_t>(deckCount_) * 52);
        for (int d = 0; d < deckCount_; ++d) {
            for (int suit = 0; suit < 4; ++suit) {
                for (int rank = 1; rank <= 13; ++rank) {
                    cards_.push_back(Card(rank, suit));
                }
            }
        }
        next_ = 0;
    }

    int deckCount_;
    std::vector<Card> cards_;
    std::size_t next_;
};

} // namespace cards

#endif // CORE_DECK_H
