/*
 * ============================================================================
 * LUCKY SPIN CASINO - A Terminal-Based Incremental Gambling Game
 * ============================================================================
 * 
 * This program demonstrates several C++ concepts:
 * - Classes and Object-Oriented Programming (OOP)
 * - Member variables and methods
 * - Constructor initialization lists
 * - ANSI escape codes for terminal colors and formatting
 * - Linux terminal input handling (termios)
 * - Random number generation
 * - Control flow (loops, conditionals, switch statements)
 * - Standard library usage (iostream, iomanip, string)
 * - Vectors (dynamic arrays) for storing game objects
 * - Time-based calculations for passive income (incremental game mechanic)
 * 
 * Compile with: g++ -std=c++11 -o gambling_game gambling_game.cpp
 * Run with: ./gambling_game
 * 
 * ============================================================================
 */

#include <iostream>
#include <stdexcept>

#include "gambling_game.h"
#include "utils.h" // terminal::init()/restore()

// ============================================================================
// MAIN FUNCTION
// ============================================================================
/*
 * main() - Program entry point
 * 
 * Every C++ program must have exactly one main() function.
 * This is where execution begins.
 * 
 * Returns: 0 indicates successful execution
 *          Non-zero values indicate errors
 */
int main() {
    /*
     * Snapshot the terminal settings BEFORE anything switches it into raw mode,
     * and register handlers to put them back. Without this, quitting with
     * Ctrl+C leaves the user's shell with no echo and no line editing.
     */
    terminal::init();
    
    try {
        // Create an instance (object) of the GamblingGame class
        // This calls the constructor, initializing all member variables
        GamblingGame game;
        
        // Try to load existing save file (Sprint 3)
        // If successful, offline earnings will be calculated and shown
        game.loadGame();
        
        // Start the game by calling the play() method
        game.play();
    }
    catch (const std::exception& e) {
        /*
         * Restore the terminal before printing, otherwise the message renders
         * into a raw-mode terminal and the shell is left unusable.
         */
        terminal::restore();
        std::cerr << "\nLucky Spin hit an unexpected error: " << e.what() << "\n";
        return 1;
    }
    catch (...) {
        terminal::restore();
        std::cerr << "\nLucky Spin hit an unexpected error.\n";
        return 1;
    }
    
    // Return 0 to indicate successful program completion
    return 0;
}
