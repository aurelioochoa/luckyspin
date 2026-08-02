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
 * spinWheel() - The wheel spinning game
 * 
 * Rules (affected by upgrades):
 * - Win chance: 50% base + WHEEL_CHANCE upgrade
 * - Payout: 2x base + WHEEL_PAYOUT upgrade
 * - Lucky Charm can double winnings
 */
void GamblingGame::spinWheel() {
    // Guard clause - exit early if can't afford bet
    if (!canAffordBet()) return;
    
    // Deduct bet from balance
    money -= betAmount;
    
    // Get upgrade values for this game (Sprint 2)
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
    
    // Generate random outcome using upgraded win chance.
    // chancePercent() honours fractional percentages exactly, unlike
    // `rand() % 100 < winChance` which rounded 62.5% up to 63%.
    bool won = Rng::global().chancePercent(winChance);
    
    cout << "\n\n";
    
    if (won) {
        // Calculate win amount using upgraded payout multiplier
        double winAmount = betAmount * payoutMult;
        
        // Apply Lucky Charm bonus if it triggers
        winAmount = applyLuckyBonus(winAmount);
        
        money += winAmount;
        totalWins++;
        updatePeakMoney();
        
        // Display win message with colors
        cout << "  " << Color::BG_GREEN << Color::BOLD << Color::WHITE 
             << "  YOU WON!  " << Color::RESET << "\n";
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
             << winAmount << Color::RESET << "\n";
    } else {
        totalLosses++;
        
        // Display loss message
        cout << "  " << Color::BG_RED << Color::BOLD << Color::WHITE 
             << "  YOU LOST  " << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
             << betAmount << Color::RESET << "\n";
    }
    
    round++;
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
 * playSlots() - The slot machine game
 * 
 * Rules (affected by upgrades):
 * - Three reels with 6 different symbols each
 * - Win chance: base ~16.7% + SLOTS_CHANCE upgrade (implemented via forced match)
 * - Payout: 5x base + SLOTS_PAYOUT upgrade
 * - Lucky Charm can double winnings
 */
void GamblingGame::playSlots() {
    if (!canAffordBet()) return;
    
    money -= betAmount;
    
    // Get upgrade values (Sprint 2)
    double winChance = getUpgradeValue(UpgradeType::SLOTS_CHANCE);
    double payoutMult = getUpgradeValue(UpgradeType::SLOTS_PAYOUT);
    
    cout << "\n  " << Color::YELLOW << "Rolling slots...";
    cout << Color::DIM << " (" << fixed << setprecision(0) << winChance 
         << "% chance, " << setprecision(1) << payoutMult << "x payout)" 
         << Color::RESET << "\n\n";
    
    // Array to store the three slot positions
    // Each element will be an index into Symbols::ALL
    int slots[3];
    
    // Animation loop - spin the slots with gradually slowing speed
    for (int spin = 0; spin < 15; spin++) {
        // Generate random symbols for each reel
        for (int i = 0; i < 3; i++) {
            slots[i] = Rng::global().below(Symbols::COUNT);
        }
        
        // \r is carriage return - moves cursor to start of line
        // This creates the animation effect by overwriting previous output
        cout << "\r    | " << getColoredSymbol(slots[0]) 
             << " | " << getColoredSymbol(slots[1]) 
             << " | " << getColoredSymbol(slots[2]) 
             << " |" << flush;
        
        // Gradually slow down the animation
        // usleep takes microseconds: 50000 = 0.05 seconds
        // As spin increases, delay increases (slowing effect)
        usleep(50000 + spin * 50000);
    }
    
    // Determine win using upgraded chance (Sprint 2)
    // Instead of relying on natural triple match (~16.7%), we use the upgrade value
    // If the random roll succeeds, we force a match; otherwise keep random result
    bool won = Rng::global().chancePercent(winChance);
    
    // If won by upgrade chance, force all slots to match
    if (won) {
        slots[1] = slots[0];
        slots[2] = slots[0];
        // Redraw the final result with matching symbols
        cout << "\r    | " << getColoredSymbol(slots[0]) 
             << " | " << getColoredSymbol(slots[1]) 
             << " | " << getColoredSymbol(slots[2]) 
             << " |" << flush;
    }
    
    cout << "\n\n";
    
    // Check for win condition: all three symbols match
    if (slots[0] == slots[1] && slots[1] == slots[2]) {
        // Calculate win with upgraded payout
        double winAmount = betAmount * payoutMult;
        
        // Apply Lucky Charm bonus if it triggers
        winAmount = applyLuckyBonus(winAmount);
        
        money += winAmount;
        totalWins++;
        updatePeakMoney();
        
        // Special jackpot display
        cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED 
             << "  *** JACKPOT! ***  " << Color::RESET << "\n";
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
             << winAmount << Color::RESET << "\n";
    } else {
        totalLosses++;
        
        cout << "  " << Color::DIM << "No match this time." << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
             << betAmount << Color::RESET << "\n";
    }
    
    round++;
    cout << "\n  Press any key to continue...";
    getch();
}

// ========================================================================
// NEW GAMES (Sprint 4 - Polish)
// ========================================================================

/*
 * playBlackjack() - Simplified Blackjack game
 * 
 * Rules:
 * - Player and dealer each get two cards
 * - Player can Hit (draw card) or Stand (keep current hand)
 * - Goal: Get as close to 21 as possible without going over
 * - Face cards (J, Q, K) = 10, Ace = 1 or 11
 * - Dealer must hit on 16 or less, stand on 17+
 * - Win pays 1.5x (Blackjack pays 2x)
 */
void GamblingGame::playBlackjack() {
    if (!canAffordBet()) return;
    
    money -= betAmount;
    
    clearScreen();
    displayHeader();
    cout << "\n";
    printLine('-', 50, Color::GREEN);
    printCentered("BLACKJACK", 50, Color::GREEN + Color::BOLD);
    printLine('-', 50, Color::GREEN);
    
    /*
     * Cards come from a real shoe (see core/deck.h): finite, shuffled, and
     * dealt without replacement, so the suit shown belongs to the card and
     * face cards keep their identity instead of all rendering as "10".
     */
    if (shoe.needsShuffle()) {
        shoe.shuffle(Rng::global());
    }
    
    auto drawCard = [&]() -> cards::Card {
        return shoe.draw(Rng::global());
    };
    
    // Render a card as [Rank+Suit], colored by its real suit
    auto displayCard = [](const cards::Card& card) -> string {
        const string suitGlyph = Cards::SUITS[card.suit];
        const string suitColor = cards::isRedSuit(card.suit) ? Color::RED : Color::WHITE;
        return "[" + suitColor + cards::rankName(card.rank) + suitGlyph + Color::RESET + "]";
    };
    
    // Deal initial cards: player, dealer, player, dealer
    vector<cards::Card> playerHand, dealerHand;
    playerHand.push_back(drawCard());
    dealerHand.push_back(drawCard());
    playerHand.push_back(drawCard());
    dealerHand.push_back(drawCard());
    
    bool playerBust = false;
    bool playerStand = false;
    
    // Player's turn with arrow/WASD navigation
    int bjSelection = 0;  // 0=Hit, 1=Stand, 2=Quit
    
    while (!playerBust && !playerStand) {
        // Clear and redraw for clean display
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::GREEN);
        printCentered(Cards::SPADE + " BLACKJACK " + Cards::HEART, 50, Color::GREEN + Color::BOLD);
        printLine('-', 50, Color::GREEN);
        
        int playerTotal = cards::handTotal(playerHand);
        
        // Display current state with card suits
        cout << "\n  " << Color::WHITE << "Dealer shows: " << Color::CYAN 
             << displayCard(dealerHand[0]) << " [" << Color::DIM << "??" << Color::RESET << "]" 
             << Color::RESET << "\n";
        
        cout << "  " << Color::WHITE << "Your hand:    " << Color::YELLOW;
        for (size_t i = 0; i < playerHand.size(); i++) {
            cout << displayCard(playerHand[i]) << " ";
        }
        cout << Color::RESET << "= " << Color::BOLD << playerTotal
             << (cards::isSoft(playerHand) ? " (soft)" : "") << Color::RESET << "\n";
        
        if (playerTotal == 21) {
            cout << "\n  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                 << " " << Cards::SPADE << " BLACKJACK! " << Cards::SPADE << " " << Color::RESET << "\n";
            playerStand = true;
            usleep(1000000);
            break;
        }
        
        if (playerTotal > 21) {
            playerBust = true;
            break;
        }
        
        // Menu-style selection for Hit/Stand/Quit
        cout << "\n  " << Color::DIM << "Use W/S or " << UI::ARROW_UP << "/" << UI::ARROW_DOWN << " to navigate, Enter to select" << Color::RESET << "\n\n";
        
        // Hit option
        if (bjSelection == 0) {
            cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Hit" << Color::RESET << "      Draw another card\n";
        } else {
            cout << "    " << Color::WHITE << "Hit" << Color::RESET << "      " << Color::DIM << "Draw another card" << Color::RESET << "\n";
        }
        
        // Stand option
        if (bjSelection == 1) {
            cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Stand" << Color::RESET << "    Keep current hand\n";
        } else {
            cout << "    " << Color::WHITE << "Stand" << Color::RESET << "    " << Color::DIM << "Keep current hand" << Color::RESET << "\n";
        }
        
        // Quit option
        if (bjSelection == 2) {
            cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " Quit" << Color::RESET << "     Forfeit bet and exit\n";
        } else {
            cout << "    " << Color::RED << "Quit" << Color::RESET << "     " << Color::DIM << "Forfeit bet and exit" << Color::RESET << "\n";
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
            bjSelection = (bjSelection - 1 + 3) % 3;
        } else if (choice == 's' || choice == 'S') {
            bjSelection = (bjSelection + 1) % 3;
        } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
            // Execute selected action
            if (bjSelection == 0) {
                // Hit
                playerHand.push_back(drawCard());
            } else if (bjSelection == 1) {
                // Stand
                playerStand = true;
            } else if (bjSelection == 2) {
                // Quit - forfeit the bet
                cout << "\n  " << Color::YELLOW << "You left the table. Bet forfeited." << Color::RESET << "\n";
                totalLosses++;
                cout << "\n  Press any key to continue...";
                getch();
                return;
            }
        } else if (choice == 'h' || choice == 'H') {
            // Quick key for Hit
            playerHand.push_back(drawCard());
        } else if (choice == 'q' || choice == 'Q') {
            // Quick key for Quit
            cout << "\n  " << Color::YELLOW << "You left the table. Bet forfeited." << Color::RESET << "\n";
            totalLosses++;
            cout << "\n  Press any key to continue...";
            getch();
            return;
        }
    }
    
    int playerTotal = cards::handTotal(playerHand);
    
    if (playerBust) {
        cout << "\n  " << Color::BG_RED << Color::WHITE << Color::BOLD 
             << " BUST! Over 21 " << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
             << betAmount << Color::RESET << "\n";
        totalLosses++;
    } else {
        // Dealer's turn
        cout << "\n  " << Color::WHITE << "Dealer reveals: " << Color::CYAN;
        for (size_t i = 0; i < dealerHand.size(); i++) {
            cout << displayCard(dealerHand[i]) << " ";
        }
        int dealerTotal = cards::handTotal(dealerHand);
        cout << Color::RESET << "= " << dealerTotal << "\n";
        usleep(800000);
        
        // Dealer draws until 17+
        // Apply Dealer Jinx upgrade - chance for dealer to bust early
        double dealerBustChance = getUpgradeValue(UpgradeType::DEALER_BUST);
        bool forcedBust = Rng::global().chancePercent(dealerBustChance);
        
        while (dealerTotal < 17) {
            cards::Card newCard = drawCard();
            dealerHand.push_back(newCard);
            dealerTotal = cards::handTotal(dealerHand);
            cout << "  Dealer draws: " << Color::CYAN << displayCard(newCard)
                 << Color::RESET << " = " << dealerTotal << "\n";
            usleep(800000);
        }
        
        // If Dealer Jinx triggered and dealer didn't naturally bust, deal the
        // dealer one more card to push them over 21.
        if (forcedBust && dealerTotal <= 21) {
            cards::Card bustCard = drawCard();
            dealerHand.push_back(bustCard);
            dealerTotal = cards::handTotal(dealerHand);
            cout << "  " << Color::MAGENTA << UI::BLACK_STAR << " Dealer Jinx! "
                 << UI::BLACK_STAR << Color::RESET << "\n";
            cout << "  Dealer draws: " << Color::CYAN << displayCard(bustCard)
                 << Color::RESET << " = " << dealerTotal << "\n";
            usleep(800000);
        }
        
        // Determine winner
        cout << "\n";
        bool dealerBust = dealerTotal > 21;
        bool playerWins = dealerBust || playerTotal > dealerTotal;
        bool push = !dealerBust && playerTotal == dealerTotal;
        
        if (dealerBust) {
            cout << "  " << Color::GREEN << "Dealer busts!" << Color::RESET << "\n";
        }
        
        if (push) {
            // Push - return bet
            money += betAmount;
            cout << "  " << Color::YELLOW << "Push! Bet returned." << Color::RESET << "\n";
        } else if (playerWins) {
            // Player wins - apply Card Counter upgrade for better payouts
            double basePayout = getUpgradeValue(UpgradeType::BLACKJACK_PAYOUT);
            double payout = cards::isNaturalBlackjack(playerHand) ? basePayout + 0.5 : basePayout;
            double winAmount = betAmount * payout;
            winAmount = applyLuckyBonus(winAmount);
            money += winAmount;
            totalWins++;
            updatePeakMoney();
            
            cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                 << " YOU WIN! " << Color::RESET << "\n";
            cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
                 << winAmount << Color::RESET << "\n";
        } else {
            // Dealer wins
            totalLosses++;
            cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD 
                 << " DEALER WINS " << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                 << betAmount << Color::RESET << "\n";
        }
    }
    
    round++;
    cout << "\n  Press any key to continue...";
    getch();
}

/*
 * playRoulette() - Roulette game with multiple bet types
 * 
 * Bet types:
 * - Red/Black: 48.6% chance, 2x payout
 * - Odd/Even: 48.6% chance, 2x payout
 * - Specific number (0-36): 2.7% chance, 35x payout
 */
void GamblingGame::playRoulette() {
    if (!canAffordBet()) return;
    
    // Red numbers on a standard roulette wheel
    int redNumbers[] = {1,3,5,7,9,12,14,16,18,19,21,23,25,27,30,32,34,36};
    int redCount = 18;
    
    auto isRed = [&](int num) -> bool {
        for (int i = 0; i < redCount; i++) {
            if (redNumbers[i] == num) return true;
        }
        return false;
    };
    
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
    
    // Deduct bet
    money -= betAmount;
    
    // Spin the wheel
    cout << "\n  " << Color::YELLOW << "Spinning the wheel";
    for (int i = 0; i < 5; i++) {
        cout << ".";
        cout.flush();
        usleep(400000);
    }
    
    // Generate result (0-36). below() is bias-free, unlike `rand() % 37`.
    int result = Rng::global().below(37);
    bool resultRed = isRed(result);
    
    cout << "\n\n  The ball lands on: ";
    if (result == 0) {
        cout << Color::BG_GREEN << Color::WHITE << Color::BOLD << " 0 " << Color::RESET;
    } else if (resultRed) {
        cout << Color::BG_RED << Color::WHITE << Color::BOLD << " " << result << " " << Color::RESET;
    } else {
        cout << Color::BG_WHITE << Color::BLACK << Color::BOLD << " " << result << " " << Color::RESET;
    }
    cout << "\n\n";
    
    // Check win - apply Croupier's Friend upgrade for better payouts
    bool won = false;
    double roulettePayout = getUpgradeValue(UpgradeType::ROULETTE_PAYOUT);
    double payout = roulettePayout;  // Base payout from upgrade
    
    // Check for Lucky 7s bonus (7, 17, 27)
    double lucky7Bonus = getUpgradeValue(UpgradeType::LUCKY_NUMBER);
    bool isLucky7 = (result == 7 || result == 17 || result == 27);
    
    switch (betType) {
        case 1: // Red
            won = (result != 0 && resultRed);
            break;
        case 2: // Black
            won = (result != 0 && !resultRed);
            break;
        case 3: // Odd
            won = (result != 0 && result % 2 == 1);
            break;
        case 4: // Even
            won = (result != 0 && result % 2 == 0);
            break;
        case 5: // Number
            won = (result == betNumber);
            payout = 35.0 + (roulettePayout - 2.0) * 5;  // Scale number payout
            break;
    }
    
    if (won) {
        double winAmount = betAmount * payout;
        
        // Apply Lucky 7s bonus if applicable
        if (isLucky7 && lucky7Bonus > 0) {
            winAmount += betAmount * lucky7Bonus;
            cout << "  " << Color::MAGENTA << "★ Lucky 7s Bonus! +" << lucky7Bonus 
                 << "x ★" << Color::RESET << "\n";
        }
        
        winAmount = applyLuckyBonus(winAmount);
        money += winAmount;
        totalWins++;
        updatePeakMoney();
        
        cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
             << " YOU WIN! " << Color::RESET << "\n";
        cout << "  " << Color::GREEN << "+$" << fixed << setprecision(2) 
             << winAmount << Color::RESET << "\n";
    } else {
        totalLosses++;
        cout << "  " << Color::BG_RED << Color::WHITE << Color::BOLD 
             << " NO WIN " << Color::RESET << "\n";
        cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
             << betAmount << Color::RESET << "\n";
    }
    
    round++;
    cout << "\n  Press any key to continue...";
    getch();
}
