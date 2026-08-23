#include <iostream>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <string>
#include <unistd.h>
#include <termios.h>
#include <iomanip>
#include <cstring>
#include <climits>
#include <vector>
#include <sys/select.h>
#include <sstream>
#include <fstream>
#include <stdexcept>

#include "gambling_game.h"
#include "colors.h"
#include "symbols.h"
#include "utils.h"
#include "core/text_width.h"
#include "core/money.h"
#include "core/rng.h"
#include "core/economy.h"
#include "core/plinko.h"
#include "core/roulette.h"
#include "core/banner.h"
#include "core/checksum.h"

// ==========================================================================
// PLAYABLE GAMES (wheel, slots, blackjack, roulette)
// ==========================================================================

/*
 * changeBet() - Allow player to modify their bet amount
 * 
 * Uses arrow/WASD navigation with preset bet amounts and custom option.
 * Demonstrates input validation - we check that the new bet is:
 * 1. Greater than 0 (can't bet nothing or negative)
 * 2. Less than or equal to current balance (can't bet more than you have)
 */
void GamblingGame::changeBet() {
    int betSelection = 0;
    bool inBetMenu = true;
    
    // Preset bet amounts
    double presets[] = {1, 5, 10, 25, 50, 100, 250, 500, 1000};
    int presetCount = 9;
    int totalOptions = presetCount + 2;  // presets + custom + back
    
    while (inBetMenu) {
        clearScreen();
        displayHeader();
        
        cout << "\n";
        printLine('-', 50, Color::YELLOW);
        printCentered("\xF0\x9F\x92\xB5 CHANGE BET \xF0\x9F\x92\xB5", 50, Color::YELLOW + Color::BOLD);
        printLine('-', 50, Color::YELLOW);
        
        cout << "\n  " << Color::WHITE << "Current bet: " << Color::CYAN 
             << "$" << fixed << setprecision(2) << betAmount << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Available balance: " << Color::GREEN << "$" << fixed 
             << setprecision(2) << money << Color::RESET << "\n";
        
        cout << "\n  " << Color::DIM << "Use W/S to navigate, Enter to select, Q to exit" << Color::RESET << "\n\n";
        
        // Display preset bet options
        for (int i = 0; i < presetCount; i++) {
            bool canAfford = (presets[i] <= money);
            bool isSelected = (betSelection == i);
            bool isCurrent = (presets[i] == betAmount);
            
            if (isSelected) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " $" << fixed << setprecision(0) << presets[i] << Color::RESET;
            } else {
                cout << "    ";
                if (canAfford) {
                    cout << Color::WHITE << "$" << fixed << setprecision(0) << presets[i] << Color::RESET;
                } else {
                    cout << Color::RED << "$" << fixed << setprecision(0) << presets[i] << Color::RESET;
                }
            }
            
            if (isCurrent) {
                cout << " " << Color::CYAN << "(current)" << Color::RESET;
            }
            if (!canAfford) {
                cout << " " << Color::DIM << "(insufficient funds)" << Color::RESET;
            }
            cout << "\n";
        }
        
        cout << "\n";
        
        // Custom amount option
        if (betSelection == presetCount) {
            cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD << " " << UI::ARROW_RIGHT << " Custom Amount" << Color::RESET << "\n";
        } else {
            cout << "    " << Color::YELLOW << "Custom Amount" << Color::RESET << "\n";
        }
        
        // Back option
        if (betSelection == presetCount + 1) {
            cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Back to Menu" << Color::RESET << "\n";
        } else {
            cout << "    " << Color::DIM << "Back to Menu" << Color::RESET << "\n";
        }
        
        char choice = getch();
        
        // Handle arrow keys
        if (choice == 27) {
            char seq1 = getchWithTimeout(50);
            if (seq1 == '[') {
                char seq2 = getchWithTimeout(50);
                if (seq2 == 'A') choice = 'w';
                else if (seq2 == 'B') choice = 's';
            }
            if (choice == 27) continue;
        }
        
        // Navigation
        if (choice == 'w' || choice == 'W') {
            betSelection = (betSelection - 1 + totalOptions) % totalOptions;
        } else if (choice == 's' || choice == 'S') {
            betSelection = (betSelection + 1) % totalOptions;
        } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
            if (betSelection == presetCount + 1) {
                // Back to menu
                inBetMenu = false;
            } else if (betSelection == presetCount) {
                // Custom amount
                cout << "\n  Enter custom bet amount: " << Color::YELLOW << "$" << Color::RESET;
                double newBet;
                cin >> newBet;
                
                if (newBet > 0 && newBet <= money) {
                    betAmount = newBet;
                    cout << "\n  " << Color::GREEN << UI::CHECK << " Bet updated to $" << fixed 
                         << setprecision(2) << betAmount << "!" << Color::RESET << "\n";
                    cout << "\n  Press any key to continue...";
                    getch();
                    inBetMenu = false;
                } else if (newBet <= 0) {
                    cout << "\n  " << Color::RED << UI::CROSS_X << " Bet must be greater than $0!" 
                         << Color::RESET << "\n";
                    cout << "\n  Press any key to continue...";
                    getch();
                } else {
                    cout << "\n  " << Color::RED << UI::CROSS_X << " Insufficient funds for that bet!" 
                         << Color::RESET << "\n";
                    cout << "\n  Press any key to continue...";
                    getch();
                }
            } else {
                // Preset amount
                double selectedBet = presets[betSelection];
                if (selectedBet <= money) {
                    betAmount = selectedBet;
                    cout << "\n  " << Color::GREEN << UI::CHECK << " Bet updated to $" << fixed 
                         << setprecision(2) << betAmount << "!" << Color::RESET << "\n";
                    cout << "\n  Press any key to continue...";
                    getch();
                    inBetMenu = false;
                } else {
                    cout << "\n  " << Color::RED << UI::CROSS_X << " Insufficient funds for that bet!" 
                         << Color::RESET << "\n";
                    cout << "\n  Press any key to continue...";
                    getch();
                }
            }
        } else if (choice == 'q' || choice == 'Q') {
            // Quick exit
            inBetMenu = false;
        }
    }
}

/*
 * spinWheel() - Terminal rendering of the wheel game
 *
 * The rules live in spinWheelRound() (game_actions.cpp) so the web GUI plays
 * exactly the same math; this function only animates and renders.
 */
void GamblingGame::spinWheel() {
    if (!canAffordBet()) return;

    double winChance = getUpgradeValue(UpgradeType::WHEEL_CHANCE);
    double payoutMult = getUpgradeValue(UpgradeType::WHEEL_PAYOUT);

    cout << "\n  " << Color::YELLOW << "Spinning the wheel";
    cout << Color::DIM << " (" << fixed << setprecision(0) << winChance
         << "% chance, " << setprecision(2) << payoutMult << "x payout)"
         << Color::RESET;

    // Animation loop - creates suspense with dots
    // cout.flush() ensures output is displayed immediately
    for (int i = 0; i < 5; i++) {
        cout << ".";
        cout.flush();  // Force output buffer to display
        usleep(300000); // Sleep for 300,000 microseconds (0.3 seconds)
    }

    const WheelRound result = spinWheelRound(betAmount);

    cout << "\n\n";

    if (result.won) {
        if (result.hotStreak) {
            cout << "  " << Color::BG_MAGENTA << Color::BOLD << Color::WHITE
                 << " HOT STREAK x1.5 " << Color::RESET << "\n";
        }
        if (result.luckyBonus) {
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
                 << " LUCKY BONUS! 2x " << Color::RESET << "\n";
        }
        cout << "  " << Color::BG_GREEN << Color::BOLD << Color::WHITE
             << "  YOU WON!  " << Color::RESET << "\n";
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2)
             << result.payout << Color::RESET << "\n";
    } else {
        cout << "  " << Color::BG_RED << Color::BOLD << Color::WHITE
             << "  YOU LOST  " << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2)
             << result.bet << Color::RESET << "\n";
    }

    cout << "\n  Press any key to continue...";
    getch();
}

/*
 * getColoredSymbol() - Get a slot symbol with its associated color
 * 
 * Parameters:
 *   - index: Index into the Symbols arrays (0 to COUNT-1)
 * 
 * Returns: A string containing the colored symbol
 * 
 * This demonstrates string concatenation with the + operator
 */
string GamblingGame::getColoredSymbol(int index) {
    return Symbols::COLORS[index] + "[" + Symbols::ALL[index] + "]" + Color::RESET;
}

/*
 * playSlots() - Terminal rendering of the slot machine
 *
 * Rules live in playSlotsRound() (game_actions.cpp); the animation below is
 * cosmetic and always ends on the reels the round actually settled with.
 */
void GamblingGame::playSlots() {
    if (!canAffordBet()) return;

    double winChance = getUpgradeValue(UpgradeType::SLOTS_CHANCE);
    double payoutMult = getUpgradeValue(UpgradeType::SLOTS_PAYOUT);

    cout << "\n  " << Color::YELLOW << "Rolling slots...";
    cout << Color::DIM << " (" << fixed << setprecision(0) << winChance
         << "% chance, " << setprecision(1) << payoutMult << "x payout)"
         << Color::RESET << "\n\n";

    // Animation loop - spin the slots with gradually slowing speed.
    // The final frame is redrawn from the settled round below.
    int slots[3] = {0, 0, 0};
    for (int spin = 0; spin < 15; spin++) {
        for (int i = 0; i < 3; i++) {
            slots[i] = Rng::global().below(Symbols::COUNT);
        }
        cout << "\r    | " << getColoredSymbol(slots[0])
             << " | " << getColoredSymbol(slots[1])
             << " | " << getColoredSymbol(slots[2])
             << " |" << flush;
        usleep(50000 + spin * 50000);
    }

    const SlotsRound result = playSlotsRound(betAmount);

    // Redraw the true final result (may match or differ from the animation).
    cout << "\r    | " << getColoredSymbol(result.reels[0])
         << " | " << getColoredSymbol(result.reels[1])
         << " | " << getColoredSymbol(result.reels[2])
         << " |" << flush;

    cout << "\n\n";

    if (result.won) {
        if (result.hotStreak) {
            cout << "  " << Color::BG_MAGENTA << Color::BOLD << Color::WHITE
                 << " HOT STREAK x1.5 " << Color::RESET << "\n";
        }
        if (result.luckyBonus) {
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
                 << " LUCKY BONUS! 2x " << Color::RESET << "\n";
        }
        cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
             << "  *** JACKPOT! ***  " << Color::RESET << "\n";
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2)
             << result.payout << Color::RESET << "\n";
    } else {
        cout << "  " << Color::DIM << "No match this time." << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2)
             << result.bet << Color::RESET << "\n";
    }

    cout << "\n  Press any key to continue...";
    getch();
}

// ========================================================================
// NEW GAMES (Sprint 4 - Polish)
// ========================================================================

/*
 * playBlackjack() - Terminal rendering of Blackjack
 *
 * Rules live in the headless state machine (game_actions.cpp): deal/hit/
 * stand drive a shared shoe, dealer AI and payouts. This function renders
 * the hands and maps keys onto the state machine.
 */
void GamblingGame::playBlackjack() {
    if (!canAffordBet()) return;
    if (!blackjackDeal(betAmount)) return;

    // Render a card as [Rank+Suit], colored by its real suit
    auto displayCard = [](const cards::Card& card) -> string {
        const string suitGlyph = Cards::SUITS[card.suit];
        const string suitColor = cards::isRedSuit(card.suit) ? Color::RED : Color::WHITE;
        return "[" + suitColor + cards::rankName(card.rank) + suitGlyph + Color::RESET + "]";
    };

    auto renderTable = [&](bool revealDealer) {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::GREEN);
        printCentered(Cards::SPADE + " BLACKJACK " + Cards::HEART, 50, Color::GREEN + Color::BOLD);
        printLine('-', 50, Color::GREEN);

        cout << "\n  " << Color::WHITE << "Dealer shows: " << Color::CYAN
             << displayCard(bjDealerCards()[0]);
        if (revealDealer) {
            for (size_t i = 1; i < bjDealerCards().size(); i++) {
                cout << " " << displayCard(bjDealerCards()[i]);
            }
            cout << Color::RESET << " = " << cards::handTotal(bjDealerCards()) << "\n";
        } else {
            cout << " [" << Color::DIM << "??" << Color::RESET << "]"
                 << Color::RESET << "\n";
        }

        const int playerTotal = cards::handTotal(bjPlayerCards());
        cout << "  " << Color::WHITE << "Your hand:    " << Color::YELLOW;
        for (size_t i = 0; i < bjPlayerCards().size(); i++) {
            cout << displayCard(bjPlayerCards()[i]) << " ";
        }
        cout << Color::RESET << "= " << Color::BOLD << playerTotal
             << (cards::isSoft(bjPlayerCards()) ? " (soft)" : "") << Color::RESET << "\n";
    };

    // Player's turn with arrow/WASD navigation
    int bjSelection = 0;  // 0=Hit, 1=Stand, 2=Quit

    while (blackjackPhase() == BlackjackPhase::PlayerTurn) {
        renderTable(false);

        const int playerTotal = cards::handTotal(bjPlayerCards());

        if (playerTotal == 21) {
            cout << "\n  " << Color::BG_GREEN << Color::WHITE << Color::BOLD
                 << " " << Cards::SPADE << " BLACKJACK! " << Cards::SPADE << " "
                 << Color::RESET << "\n";
            usleep(1000000);
            break;  // nothing left to draw - straight to the dealer
        }

        // Menu-style selection for Hit/Stand/Quit
        cout << "\n  " << Color::DIM << "Use W/S or " << UI::ARROW_UP << "/"
             << UI::ARROW_DOWN << " to navigate, Enter to select" << Color::RESET << "\n\n";

        static const char* labels[3] = {"Hit", "Stand", "Quit"};
        static const char* hints[3] = {"Draw another card", "Keep current hand",
                                       "Forfeit bet and exit"};
        for (int i = 0; i < 3; i++) {
            if (bjSelection == i) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " "
                     << UI::ARROW_RIGHT << " " << labels[i] << Color::RESET
                     << "    " << hints[i] << "\n";
            } else {
                cout << "    " << (i == 2 ? Color::RED : Color::WHITE) << labels[i]
                     << Color::RESET << "    " << Color::DIM << hints[i]
                     << Color::RESET << "\n";
            }
        }

        char choice = getch();

        // Handle arrow keys
        if (choice == 27) {
            char seq1 = getchWithTimeout(50);
            if (seq1 == '[') {
                char seq2 = getchWithTimeout(50);
                if (seq2 == 'A') choice = 'w';
                else if (seq2 == 'B') choice = 's';
            }
            if (choice == 27) continue;
        }

        if (choice == 'w' || choice == 'W') {
            bjSelection = (bjSelection - 1 + 3) % 3;
        } else if (choice == 's' || choice == 'S') {
            bjSelection = (bjSelection + 1) % 3;
        } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
            if (bjSelection == 0) {
                blackjackHit();               // may bust and settle
            } else if (bjSelection == 1) {
                blackjackStand();
            } else {
                blackjackForfeit();
                cout << "\n  " << Color::YELLOW
                     << "You left the table. Bet forfeited." << Color::RESET << "\n";
                cout << "\n  Press any key to continue...";
                getch();
                return;
            }
        } else if (choice == 'h' || choice == 'H') {
            blackjackHit();
        } else if (choice == 'q' || choice == 'Q') {
            blackjackForfeit();
            cout << "\n  " << Color::YELLOW << "You left the table. Bet forfeited."
                 << Color::RESET << "\n";
            cout << "\n  Press any key to continue...";
            getch();
            return;
        }
    }

    // Hand is settled: reveal the dealer and the outcome. If the player
    // broke out of the loop on a natural 21, the dealer still plays out.
    if (bjLastOutcome() == BlackjackOutcome::None) {
        blackjackStand();
    }
    const BlackjackSettlement& s = lastSettlement();

    renderTable(true);
    cout << "\n";

    switch (s.outcome) {
        case BlackjackOutcome::PlayerBust:
            cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD
                 << " BUST! Over 21 " << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2)
                 << s.bet << Color::RESET << "\n";
            break;
        case BlackjackOutcome::Forfeited:
            cout << "  " << Color::YELLOW << "Bet forfeited." << Color::RESET << "\n";
            break;
        default: {
            for (size_t i = 0; i < s.dealerDraws.size(); i++) {
                cout << "  Dealer draws: " << Color::CYAN
                     << displayCard(s.dealerDraws[i]) << Color::RESET << "\n";
                usleep(400000);
            }
            if (s.jinx) {
                cout << "  " << Color::MAGENTA << UI::BLACK_STAR << " Dealer Jinx! "
                     << UI::BLACK_STAR << Color::RESET << "\n";
            }
            if (s.dealerBust) {
                cout << "  " << Color::GREEN << "Dealer busts!" << Color::RESET << "\n";
            }

            if (s.outcome == BlackjackOutcome::Push) {
                cout << "  " << Color::YELLOW << "Push! Bet returned." << Color::RESET << "\n";
            } else if (s.outcome == BlackjackOutcome::Win) {
                if (s.hotStreak) {
                    cout << "  " << Color::BG_MAGENTA << Color::BOLD << Color::WHITE
                         << " HOT STREAK x1.5 " << Color::RESET << "\n";
                }
                if (s.luckyBonus) {
                    cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
                         << " LUCKY BONUS! 2x " << Color::RESET << "\n";
                }
                cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD
                     << " YOU WIN! " << Color::RESET << "\n";
                cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2)
                     << s.payout << Color::RESET << "\n";
            } else {
                cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD
                     << " DEALER WINS " << Color::RESET << "\n";
                cout << "  " << Color::RED << "-$" << fixed << setprecision(2)
                     << s.bet << Color::RESET << "\n";
            }
            break;
        }
    }

    cout << "\n  Press any key to continue...";
    getch();
}

/*
 * playRoulette() - Terminal rendering of Roulette
 *
 * Rules live in playRouletteRound() / core/roulette.h; the wheel layout,
 * win logic and payout formulas are shared with the web GUI.
 */
void GamblingGame::playRoulette() {
    if (!canAffordBet()) return;

    // Menu-style bet selection with arrow/WASD navigation
    int rouletteSelection = 0;  // 0=Red, 1=Black, 2=Odd, 3=Even, 4=Number, 5=Cancel
    int betType = -1;
    int betNumber = -1;
    
    while (betType == -1) {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::RED);
        printCentered("\xF0\x9F\x8E\xB2 ROULETTE \xF0\x9F\x8E\xB2", 50, Color::RED + Color::BOLD);
        printLine('-', 50, Color::RED);
        
        cout << "\n  " << Color::WHITE << "Balance: " << Color::GREEN 
             << formatMoney(money) << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Bet: " << Color::CYAN << "$" << fixed 
             << setprecision(2) << betAmount << Color::RESET << "\n";
        
        cout << "\n  " << Color::DIM << "Use W/S or " << UI::ARROW_UP << "/" << UI::ARROW_DOWN << " to navigate, Enter to select" << Color::RESET << "\n\n";
        
        // Menu options
        struct RouletteOption {
            string name;
            string desc;
            string color;
        };
        
        RouletteOption options[] = {
            {"Red", "2x payout", Color::RED},
            {"Black", "2x payout", Color::WHITE},
            {"Odd", "2x payout", Color::CYAN},
            {"Even", "2x payout", Color::CYAN},
            {"Number", "35x payout", Color::YELLOW},
            {"Cancel", "Return to menu", Color::DIM}
        };
        
        for (int i = 0; i < 6; i++) {
            if (i == rouletteSelection) {
                if (i == 5) {
                    // Cancel option - red background
                    cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " " << options[i].name << Color::RESET;
                } else {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " " << options[i].color << options[i].name << Color::RESET;
                }
                cout << "    " << options[i].desc << "\n";
            } else {
                cout << "    " << options[i].color << options[i].name << Color::RESET;
                cout << "    " << Color::DIM << options[i].desc << Color::RESET << "\n";
            }
        }
        
        char choice = getch();
        
        // Handle arrow keys
        if (choice == 27) {
            char seq1 = getchWithTimeout(50);
            if (seq1 == '[') {
                char seq2 = getchWithTimeout(50);
                if (seq2 == 'A') choice = 'w';
                else if (seq2 == 'B') choice = 's';
            }
            if (choice == 27) continue;
        }
        
        // Navigation
        if (choice == 'w' || choice == 'W') {
            rouletteSelection = (rouletteSelection - 1 + 6) % 6;
        } else if (choice == 's' || choice == 'S') {
            rouletteSelection = (rouletteSelection + 1) % 6;
        } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
            if (rouletteSelection == 5) {
                // Cancel
                return;
            } else if (rouletteSelection == 4) {
                // Number bet - need to get the number
                cout << "\n  Enter number (0-36): ";
                cin >> betNumber;
                if (betNumber < 0 || betNumber > 36) {
                    cout << "  " << Color::RED << "Invalid number!" << Color::RESET << "\n";
                    usleep(1000000);
                    betNumber = -1;
                    continue;
                }
                betType = 5;
            } else {
                betType = rouletteSelection + 1;  // 1=Red, 2=Black, 3=Odd, 4=Even
            }
        } else if (choice == 'q' || choice == 'Q') {
            // Quick cancel
            return;
        }
    }
    
    // Deduct the bet and settle the round headlessly.
    const RouletteRound result = playRouletteRound(betAmount, betType, betNumber);
    if (!result.played) return;   // defensive: guards above should prevent this

    // Spin the wheel
    cout << "\n  " << Color::YELLOW << "Spinning the wheel";
    for (int i = 0; i < 5; i++) {
        cout << ".";
        cout.flush();
        usleep(400000);
    }

    const int result_ = result.result;
    const bool resultRed = roulette::isRed(result_);

    cout << "\n\n  The ball lands on: ";
    if (result_ == 0) {
        cout << Color::BG_GREEN << Color::WHITE << Color::BOLD << " 0 " << Color::RESET;
    } else if (resultRed) {
        cout << Color::BG_RED << Color::WHITE << Color::BOLD << " " << result_ << " " << Color::RESET;
    } else {
        cout << Color::BG_WHITE << Color::BLACK << Color::BOLD << " " << result_ << " " << Color::RESET;
    }
    cout << "\n\n";

    if (result.won) {
        if (result.luckySeven) {
            cout << "  " << Color::MAGENTA << "\xE2\x98\x85 Lucky 7s Bonus! +"
                 << fixed << setprecision(1)
                 << getUpgradeValue(UpgradeType::LUCKY_NUMBER) << "x \xE2\x98\x85"
                 << Color::RESET << "\n";
        }
        if (result.hotStreak) {
            cout << "  " << Color::BG_MAGENTA << Color::BOLD << Color::WHITE
                 << " HOT STREAK x1.5 " << Color::RESET << "\n";
        }
        if (result.luckyBonus) {
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
                 << " LUCKY BONUS! 2x " << Color::RESET << "\n";
        }
        cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD
             << " YOU WIN! " << Color::RESET << "\n";
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2)
             << result.payout << Color::RESET << "\n";
    } else {
        cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD
             << " NO WIN " << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2)
             << result.bet << Color::RESET << "\n";
    }

    cout << "\n  Press any key to continue...";
    getch();
}

/*
 * playPlinko() - Terminal rendering of Plinko
 *
 * Board math lives in core/plinko.h and dropPlinkoBall() (game_actions.cpp);
 * the drop is settled first, then animated along its real path.
 */
void GamblingGame::playPlinko() {
    if (!canAffordBet()) return;

    // Risk selection menu
    const char* riskNames[plinko::RISK_COUNT] = {"Low Risk", "Medium Risk", "High Risk"};
    const char* riskDesc[plinko::RISK_COUNT] = {
        "Steady returns, small jackpots (2x edge)",
        "Balanced spread, decent jackpots (6x edge)",
        "Mostly losses, huge jackpots (35x edge)"
    };

    int selection = 0;  // 0-2 risk levels, 3 = cancel
    int riskChoice = -1;

    while (riskChoice == -1) {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::MAGENTA);
        printCentered("\xF0\x9F\x8E\xAF PLINKO \xF0\x9F\x8E\xAF", 50, Color::MAGENTA + Color::BOLD);
        printLine('-', 50, Color::MAGENTA);

        cout << "\n  " << Color::WHITE << "Balance: " << Color::GREEN
             << formatMoney(money) << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Bet: " << Color::CYAN << "$" << fixed
             << setprecision(2) << betAmount << Color::RESET << "\n";

        cout << "\n  " << Color::DIM << "Use W/S or " << UI::ARROW_UP << "/" << UI::ARROW_DOWN
             << " to navigate, Enter to drop the ball" << Color::RESET << "\n\n";

        for (int i = 0; i < plinko::RISK_COUNT; i++) {
            if (i == selection) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " "
                     << UI::ARROW_RIGHT << " " << riskNames[i] << Color::RESET;
                cout << "    " << riskDesc[i] << "\n";
            } else {
                cout << "    " << Color::WHITE << riskNames[i] << Color::RESET;
                cout << "    " << Color::DIM << riskDesc[i] << Color::RESET << "\n";
            }
        }

        if (selection == plinko::RISK_COUNT) {
            cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " "
                 << UI::ARROW_RIGHT << " Cancel" << Color::RESET << "    Return to menu\n";
        } else {
            cout << "    " << Color::RED << "Cancel" << Color::RESET << "    "
                 << Color::DIM << "Return to menu" << Color::RESET << "\n";
        }

        char choice = getch();

        if (choice == 27) {
            char seq1 = getchWithTimeout(50);
            if (seq1 == '[') {
                char seq2 = getchWithTimeout(50);
                if (seq2 == 'A') choice = 'w';
                else if (seq2 == 'B') choice = 's';
            }
            if (choice == 27) continue;
        }

        if (choice == 'w' || choice == 'W') {
            selection = (selection - 1 + plinko::RISK_COUNT + 1) % (plinko::RISK_COUNT + 1);
        } else if (choice == 's' || choice == 'S') {
            selection = (selection + 1) % (plinko::RISK_COUNT + 1);
        } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
            if (selection == plinko::RISK_COUNT) return;
            riskChoice = selection;
        } else if (choice == 'q' || choice == 'Q') {
            return;
        }
    }

    const plinko::Risk risk = (plinko::Risk)riskChoice;

    // Settle the drop headlessly, then animate its real path.
    const PlinkoRound result = dropPlinkoBall(betAmount, riskChoice);
    if (!result.played) return;

    // Animate the drop, one board row at a time
    clearScreen();
    displayHeader();
    cout << "\n";
    printLine('-', 50, Color::MAGENTA);
    printCentered("PLINKO - " + string(riskNames[riskChoice]), 50, Color::MAGENTA + Color::BOLD);
    printLine('-', 50, Color::MAGENTA);
    cout << "\n";

    for (int r = 1; r <= plinko::ROWS; r++) {
        // Indent so each row centers under the previous one
        for (int pad = 0; pad < plinko::ROWS - r + 6; pad++) cout << " ";
        for (int i = 0; i <= r; i++) {
            if (i == result.path[r]) {
                cout << Color::YELLOW << Color::BOLD << "o" << Color::RESET;
            } else {
                cout << Color::DIM << "." << Color::RESET;
            }
            cout << " ";
        }
        cout << "\n";
        cout.flush();
        usleep(180000);
    }

    // Show the bucket row with the landing bucket highlighted
    const double* table = plinko::multiplierTable(risk);
    cout << "\n  ";
    for (int k = 0; k < plinko::BUCKETS; k++) {
        ostringstream label;
        label << fixed << setprecision(table[k] < 1.0 ? 1 : 0) << table[k];
        string text = label.str();
        const int width = 5;
        int padLeft = (width - (int)text.size()) / 2;
        if (padLeft < 0) padLeft = 0;
        int padRight = width - padLeft - (int)text.size();
        if (padRight < 0) padRight = 0;
        if (k == result.bucket) {
            cout << string(padLeft, ' ') << Color::BG_YELLOW << Color::BLACK << Color::BOLD
                 << text << Color::RESET << string(padRight, ' ');
        } else {
            cout << string(padLeft, ' ') << Color::DIM << text << Color::RESET
                 << string(padRight, ' ');
        }
    }
    cout << "\n\n";

    const bool edge = result.jackpot;

    if (result.payout > 0) {
        if (result.hotStreak) {
            cout << "  " << Color::BG_MAGENTA << Color::BOLD << Color::WHITE
                 << " HOT STREAK x1.5 " << Color::RESET << "\n";
        }
        if (result.luckyBonus) {
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
                 << " LUCKY BONUS! 2x " << Color::RESET << "\n";
        }
        if (edge) {
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED
                 << "  *** PLINKO JACKPOT! ***  " << Color::RESET << "\n";
        } else if (result.payout >= result.bet) {
            cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD
                 << " YOU WIN! " << Color::RESET << "\n";
        } else {
            cout << "  " << Color::YELLOW << "Partial return." << Color::RESET << "\n";
        }
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2)
             << result.payout << Color::RESET << "\n";
    } else {
        cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD
             << " NO RETURN " << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2)
             << result.bet << Color::RESET << "\n";
    }

    cout << "\n  Press any key to continue...";
    getch();
}
