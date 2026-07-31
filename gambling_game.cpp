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

// ============================================================================
// HEADER INCLUDES
// ============================================================================
// Each #include brings in a "header file" containing declarations of functions,
// classes, and constants that we can use in our program.

#include <iostream>   // For input/output streams (cout, cin, endl)
#include <cstdlib>    // For rand(), srand() - random number generation
#include <cmath>      // For pow() - power function
#include <ctime>      // For time() - used to seed random number generator
#include <string>     // For std::string class
#include <unistd.h>   // For usleep() - microsecond delays (POSIX/Linux)
#include <termios.h>  // For terminal I/O control (Linux-specific)
#include <iomanip>    // For setprecision(), fixed - number formatting
#include <cstring>    // For memset() - memory initialization
#include <climits>    // For INT_MAX
#include <vector>     // For std::vector - dynamic arrays
#include <sys/select.h> // For select() - non-blocking input check
#include <sstream>    // For ostringstream - string building
#include <fstream>    // For file I/O (save/load game) - Sprint 3

// Using the standard namespace so we don't have to write "std::" everywhere
// In larger projects, it's often better to use "std::" explicitly
using namespace std;

// ============================================================================
// ANSI COLOR CODES
// ============================================================================
// ANSI escape codes are special character sequences that terminals interpret
// as commands rather than text. They start with ESC (escape character, \033)
// followed by '[' and then the command.
// 
// Format: \033[<code>m
// - \033 is the escape character (octal notation)
// - [ starts the control sequence
// - <code> is the color/style code
// - m ends the sequence
//
// These are defined as constants using 'const' keyword, which means their
// values cannot be changed after initialization. This is safer than #define.

namespace Color {
    // Reset - turns off all formatting
    const string RESET   = "\033[0m";
    
    // Regular text colors (foreground)
    const string RED     = "\033[31m";    // Used for losses, errors
    const string GREEN   = "\033[32m";    // Used for wins, success
    const string YELLOW  = "\033[33m";    // Used for warnings, gold/money
    const string BLUE    = "\033[34m";    // Used for information
    const string MAGENTA = "\033[35m";    // Used for special items
    const string CYAN    = "\033[36m";    // Used for highlights
    const string WHITE   = "\033[37m";    // Default text
    
    // Bold/bright versions (add intensity)
    const string BOLD    = "\033[1m";     // Makes text bold
    const string DIM     = "\033[2m";     // Makes text dimmer
    
    // Background colors
    const string BG_RED     = "\033[41m";
    const string BG_GREEN   = "\033[42m";
    const string BG_YELLOW  = "\033[43m";
    const string BG_BLUE    = "\033[44m";
    const string BG_WHITE   = "\033[47m";
    
    // Additional foreground color
    const string BLACK      = "\033[30m";
}

// ============================================================================
// SLOT MACHINE SYMBOLS (Unicode - Sprint 6)
// ============================================================================
// Using Unicode symbols via escape codes for better portability.
// UTF-8 encoding: 4-byte emoji use \xF0\x9F\xXX\xXX format

namespace Symbols {
    // Symbol characters - UTF-8 escape sequences
    const string CHERRY  = "\xF0\x9F\x8D\x92";  // U+1F352 CHERRIES
    const string ORANGE  = "\xF0\x9F\x8D\x8A";  // U+1F34A TANGERINE
    const string LEMON   = "\xF0\x9F\x8D\x8B";  // U+1F34B LEMON
    const string GRAPE   = "\xF0\x9F\x8D\x87";  // U+1F347 GRAPES
    const string BELL    = "\xF0\x9F\x94\x94";  // U+1F514 BELL
    const string SEVEN   = "7\xEF\xB8\x8F\xE2\x83\xA3";  // 7️⃣ KEYCAP SEVEN
    const string DIAMOND = "\xF0\x9F\x92\x8E";  // U+1F48E GEM STONE
    const string STAR    = "\xE2\xAD\x90";      // U+2B50 WHITE MEDIUM STAR
    
    // Array of all symbols for easy random selection
    const string ALL[] = {CHERRY, ORANGE, LEMON, GRAPE, BELL, SEVEN, DIAMOND, STAR};
    
    // Corresponding colors for each symbol (used for background effects)
    const string COLORS[] = {
        Color::RED,      // Cherry - red
        Color::YELLOW,   // Orange - yellow/orange
        Color::YELLOW,   // Lemon - yellow
        Color::MAGENTA,  // Grape - purple/magenta
        Color::CYAN,     // Bell - cyan
        Color::GREEN,    // Seven - green (lucky!)
        Color::CYAN,     // Diamond - cyan
        Color::YELLOW    // Star - yellow
    };
    
    // Total number of different symbols
    const int COUNT = sizeof(ALL) / sizeof(ALL[0]);
}

// ============================================================================
// CARD SYMBOLS (Unicode - Sprint 6)
// ============================================================================
// Card suit symbols using UTF-8 escape codes

namespace Cards {
    // UTF-8: E2 99 XX for card suits (U+2660 block)
    const string SPADE   = "\xE2\x99\xA0";  // U+2660 BLACK SPADE SUIT
    const string HEART   = "\xE2\x99\xA5";  // U+2665 BLACK HEART SUIT
    const string DIAMOND = "\xE2\x99\xA6";  // U+2666 BLACK DIAMOND SUIT
    const string CLUB    = "\xE2\x99\xA3";  // U+2663 BLACK CLUB SUIT
    
    // Suit colors
    const string SPADE_COLOR   = Color::WHITE;
    const string HEART_COLOR   = Color::RED;
    const string DIAMOND_COLOR = Color::RED;
    const string CLUB_COLOR    = Color::WHITE;
}

// ============================================================================
// UI DECORATIONS (Unicode - Sprint 6)
// ============================================================================
// Box drawing and UI icons using UTF-8 escape codes

namespace UI {
    // Box drawing characters (U+2500 block) - UTF-8: E2 94 XX
    const string H_LINE    = "\xE2\x94\x80";  // U+2500 BOX DRAWINGS LIGHT HORIZONTAL
    const string V_LINE    = "\xE2\x94\x82";  // U+2502 BOX DRAWINGS LIGHT VERTICAL
    const string TL_CORNER = "\xE2\x94\x8C";  // U+250C BOX DRAWINGS LIGHT DOWN AND RIGHT
    const string TR_CORNER = "\xE2\x94\x90";  // U+2510 BOX DRAWINGS LIGHT DOWN AND LEFT
    const string BL_CORNER = "\xE2\x94\x94";  // U+2514 BOX DRAWINGS LIGHT UP AND RIGHT
    const string BR_CORNER = "\xE2\x94\x98";  // U+2518 BOX DRAWINGS LIGHT UP AND LEFT
    const string T_JOIN    = "\xE2\x94\xAC";  // U+252C BOX DRAWINGS LIGHT DOWN AND HORIZONTAL
    const string B_JOIN    = "\xE2\x94\xB4";  // U+2534 BOX DRAWINGS LIGHT UP AND HORIZONTAL
    const string L_JOIN    = "\xE2\x94\x9C";  // U+251C BOX DRAWINGS LIGHT VERTICAL AND RIGHT
    const string R_JOIN    = "\xE2\x94\xA4";  // U+2524 BOX DRAWINGS LIGHT VERTICAL AND LEFT
    const string CROSS     = "\xE2\x94\xBC";  // U+253C BOX DRAWINGS LIGHT VERTICAL AND HORIZONTAL
    
    // Stars (U+2605 block) - UTF-8: E2 98 XX
    const string BLACK_STAR = "\xE2\x98\x85";  // U+2605 BLACK STAR
    const string WHITE_STAR = "\xE2\x98\x86";  // U+2606 WHITE STAR
    
    // Arrows/triangles (U+25XX block)
    const string ARROW_RIGHT = "\xE2\x96\xB6";  // U+25B6 BLACK RIGHT-POINTING TRIANGLE
    const string ARROW_UP    = "\xE2\x96\xB2";  // U+25B2 BLACK UP-POINTING TRIANGLE
    const string ARROW_DOWN  = "\xE2\x96\xBC";  // U+25BC BLACK DOWN-POINTING TRIANGLE
    const string ARROW_LEFT  = "\xE2\x97\x80";  // U+25C0 BLACK LEFT-POINTING TRIANGLE
    
    // Check marks (U+27XX block)
    const string CHECK     = "\xE2\x9C\x93";  // U+2713 CHECK MARK
    const string CROSS_X   = "\xE2\x9C\x97";  // U+2717 BALLOT X
    
    // Emoji icons (4-byte UTF-8)
    const string MONEY     = "\xF0\x9F\x92\xB0";  // U+1F4B0 MONEY BAG
    const string TROPHY    = "\xF0\x9F\x8F\x86";  // U+1F3C6 TROPHY
    const string DICE      = "\xF0\x9F\x8E\xB2";  // U+1F3B2 GAME DIE
    const string CHART     = "\xF0\x9F\x93\x88";  // U+1F4C8 CHART INCREASING
    const string CLOCK     = "\xE2\x8F\xB0";      // U+23F0 ALARM CLOCK
    const string STAR      = "\xE2\xAD\x90";      // U+2B50 WHITE MEDIUM STAR
    const string FIRE      = "\xF0\x9F\x94\xA5";  // U+1F525 FIRE
    const string SLOT      = "\xF0\x9F\x8E\xB0";  // U+1F3B0 SLOT MACHINE
    const string WHEEL     = "\xF0\x9F\x8E\xA1";  // U+1F3A1 FERRIS WHEEL
    const string CARD      = "\xF0\x9F\x83\x8F";  // U+1F0CF PLAYING CARD BLACK JOKER
    const string STORE     = "\xF0\x9F\x8F\xAA";  // U+1F3EA CONVENIENCE STORE
    const string DOLLAR    = "\xF0\x9F\x92\xB5";  // U+1F4B5 DOLLAR BANKNOTE
    const string FLOPPY    = "\xF0\x9F\x92\xBE";  // U+1F4BE FLOPPY DISK
    const string DOOR      = "\xF0\x9F\x9A\xAA";  // U+1F6AA DOOR
    const string CROWN     = "\xF0\x9F\x91\x91";  // U+1F451 CROWN
    const string GLOBE     = "\xF0\x9F\x8C\x8D";  // U+1F30D EARTH GLOBE
    const string GEM       = "\xF0\x9F\x92\x8E";  // U+1F48E GEM STONE
    const string JAR       = "\xF0\x9F\xAB\x99";  // U+1FAD9 JAR
    const string BUILDING  = "\xF0\x9F\x8F\x9B";  // U+1F3DB CLASSICAL BUILDING
    const string UP_ARROW  = "\xE2\xAC\x86";      // U+2B06 UPWARDS BLACK ARROW
}

// ============================================================================
// GENERATOR CLASS (Passive Income)
// ============================================================================
/*
 * Generator - Represents a passive income source in the incremental game
 * 
 * This is the core mechanic of incremental/idle games. Generators produce
 * money automatically over time, even when the player isn't actively playing.
 * 
 * Key concepts demonstrated:
 * - Class with multiple member variables
 * - Getter methods (accessors) for encapsulation
 * - Cost scaling formula common in incremental games
 */
class Generator {
private:
    string name;           // Display name of the generator
    string shortName;      // Short 2-3 letter code for compact display
    string description;    // Flavor text describing the generator (Sprint 5)
    string icon;           // Unicode icon for visual display (Sprint 6)
    double baseIncome;     // Base income per second when you own 1
    double baseCost;       // Cost to buy the first one
    double costMultiplier; // Each purchase multiplies cost by this amount
    int owned;             // How many of this generator the player owns
    
public:
    /*
     * Constructor with initialization list
     * 
     * All parameters are passed to initialize the generator's properties.
     * The 'owned' count starts at 0 - player must purchase generators.
     */
    Generator(const string& n, const string& sn, const string& desc, const string& ico,
              double income, double cost, double mult)
        : name(n), shortName(sn), description(desc), icon(ico), baseIncome(income), 
          baseCost(cost), costMultiplier(mult), owned(0) {}
    
    // ========================================================================
    // GETTER METHODS (Accessors)
    // ========================================================================
    /*
     * Getters provide read-only access to private member variables.
     * This is called "encapsulation" - we control how data is accessed.
     * 
     * The 'const' after the function name means this method doesn't
     * modify any member variables (it's a "const member function").
     */
    
    string getName() const { return name; }
    string getShortName() const { return shortName; }
    string getDescription() const { return description; }
    string getIcon() const { return icon; }
    int getOwned() const { return owned; }
    double getBaseIncome() const { return baseIncome; }
    double getCostMultiplier() const { return costMultiplier; }
    
    /*
     * getCurrentCost() - Calculate the cost to buy the next generator
     * 
     * This uses the standard incremental game cost formula:
     *   cost = baseCost * (costMultiplier ^ owned)
     * 
     * Example with baseCost=10, costMultiplier=1.15:
     *   - 1st purchase: 10 * 1.15^0 = 10.00
     *   - 2nd purchase: 10 * 1.15^1 = 11.50
     *   - 3rd purchase: 10 * 1.15^2 = 13.23
     *   - etc.
     * 
     * The pow() function raises a number to a power (from <cmath>, included via <cstdlib>)
     */
    double getCurrentCost() const {
        return baseCost * pow(costMultiplier, owned);
    }
    
    /*
     * getIncomePerSecond() - Calculate total income from all owned generators
     * 
     * Simple formula: baseIncome * owned
     * If you own 5 generators that each produce $0.1/sec, you get $0.5/sec
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
     * Simply increments the owned count. The actual money deduction
     * is handled by the game class to keep responsibilities separate.
     */
    void purchase() {
        owned++;
    }
    
    /*
     * setOwned() - Set the owned count directly (for loading saved games)
     * 
     * This is used by the save/load system to restore generator state.
     * Normally we'd use purchase() for incremental changes.
     */
    void setOwned(int count) {
        owned = count;
    }
    
    /*
     * reset() - Reset generator to initial state (for game restart)
     */
    void reset() {
        owned = 0;
    }
};

// ============================================================================
// UPGRADE CLASS (Sprint 2 - Game Progression)
// ============================================================================
/*
 * Upgrade - Represents a purchasable improvement to game mechanics
 * 
 * Upgrades differ from generators in that they modify game rules rather than
 * producing passive income. Examples:
 * - Increase win chance on wheel spin (50% -> 55% -> 60%)
 * - Increase payout multipliers (2x -> 2.5x -> 3x)
 * - Boost passive income from all generators
 * 
 * Key concepts demonstrated:
 * - Enum class for type-safe categorization
 * - Multiple upgrade levels with different costs/values
 * - Strategy pattern (different upgrade types affect different mechanics)
 */

/*
 * UpgradeType - Enum class to categorize different upgrade types
 * 
 * Enum classes (C++11) are type-safe enumerations. Unlike plain enums,
 * they don't implicitly convert to integers and their values are scoped.
 * 
 * Usage: UpgradeType::WHEEL_CHANCE instead of just WHEEL_CHANCE
 */
enum class UpgradeType {
    WHEEL_CHANCE,      // Increases wheel spin win chance
    WHEEL_PAYOUT,      // Increases wheel spin payout multiplier
    SLOTS_CHANCE,      // Increases slots win chance (via weighted symbols)
    SLOTS_PAYOUT,      // Increases slots payout multiplier
    INCOME_BOOST,      // Multiplies all passive income
    LUCKY_BONUS,       // Chance for bonus multiplier on any win
    // Sprint 5: Blackjack upgrades
    BLACKJACK_PAYOUT,  // Increases blackjack win payout
    DEALER_BUST,       // Increases chance dealer busts
    // Sprint 5: Roulette upgrades
    ROULETTE_PAYOUT,   // Increases roulette payout multiplier
    LUCKY_NUMBER       // Bonus payout on specific numbers
};

class Upgrade {
private:
    string name;              // Display name
    string description;       // What this upgrade does
    UpgradeType type;         // Category of upgrade
    int currentLevel;         // Current upgrade level (0 = not purchased)
    int maxLevel;             // Maximum level this upgrade can reach
    double baseCost;          // Cost for level 1
    double costMultiplier;    // Cost increase per level
    double baseValue;         // Starting value (e.g., 50% for wheel chance)
    double valuePerLevel;     // How much the value increases per level
    
public:
    /*
     * Constructor - Initialize all upgrade properties
     * 
     * Parameters:
     *   - n: Display name
     *   - desc: Description of what the upgrade does
     *   - t: Type of upgrade (from UpgradeType enum)
     *   - maxLvl: Maximum level achievable
     *   - cost: Base cost for first level
     *   - costMult: How much cost increases per level
     *   - baseVal: Starting value before any upgrades
     *   - valPerLvl: How much value increases per level
     */
    Upgrade(const string& n, const string& desc, UpgradeType t, 
            int maxLvl, double cost, double costMult, 
            double baseVal, double valPerLvl)
        : name(n), description(desc), type(t), currentLevel(0), maxLevel(maxLvl),
          baseCost(cost), costMultiplier(costMult), 
          baseValue(baseVal), valuePerLevel(valPerLvl) {}
    
    // ========================================================================
    // GETTER METHODS
    // ========================================================================
    
    string getName() const { return name; }
    string getDescription() const { return description; }
    UpgradeType getType() const { return type; }
    int getCurrentLevel() const { return currentLevel; }
    int getMaxLevel() const { return maxLevel; }
    
    /*
     * isMaxed() - Check if upgrade is at maximum level
     * 
     * Returns: true if no more levels can be purchased
     */
    bool isMaxed() const {
        return currentLevel >= maxLevel;
    }
    
    /*
     * getCurrentCost() - Get cost for the next level
     * 
     * Formula: baseCost * (costMultiplier ^ currentLevel)
     * Returns 0 if already maxed (can't purchase)
     */
    double getCurrentCost() const {
        if (isMaxed()) return 0;
        return baseCost * pow(costMultiplier, currentLevel);
    }
    
    /*
     * getCurrentValue() - Get the current effect value
     * 
     * Formula: baseValue + (valuePerLevel * currentLevel)
     * 
     * Example for wheel chance:
     *   baseValue = 50 (50% base chance)
     *   valuePerLevel = 2 (each level adds 2%)
     *   Level 0: 50 + (2 * 0) = 50%
     *   Level 1: 50 + (2 * 1) = 52%
     *   Level 2: 50 + (2 * 2) = 54%
     */
    double getCurrentValue() const {
        return baseValue + (valuePerLevel * currentLevel);
    }
    
    /*
     * getNextValue() - Preview what the value will be after upgrading
     * 
     * Useful for showing players what they'll get before purchasing
     */
    double getNextValue() const {
        if (isMaxed()) return getCurrentValue();
        return baseValue + (valuePerLevel * (currentLevel + 1));
    }
    
    // ========================================================================
    // MODIFIER METHODS
    // ========================================================================
    
    /*
     * purchase() - Upgrade to the next level
     * 
     * Returns: true if upgrade was successful, false if already maxed
     */
    bool purchase() {
        if (isMaxed()) return false;
        currentLevel++;
        return true;
    }
    
    /*
     * setLevel() - Set the upgrade level directly (for loading saved games)
     * 
     * Clamps the value to valid range [0, maxLevel].
     */
    void setLevel(int level) {
        if (level < 0) level = 0;
        if (level > maxLevel) level = maxLevel;
        currentLevel = level;
    }
    
    /*
     * reset() - Reset upgrade to initial state (for game restart)
     */
    void reset() {
        currentLevel = 0;
    }
};

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/*
 * getch() - Get a single character from the terminal without waiting for Enter
 * 
 * This function modifies terminal settings temporarily to:
 * 1. Disable canonical mode (line buffering) - normally terminal waits for Enter
 * 2. Disable echo - don't show the character the user types
 * 
 * This is Linux-specific and uses the termios structure to control terminal
 * behavior. On Windows, you would use _getch() from <conio.h> instead.
 * 
 * Returns: The character that was pressed
 */
char getch() {
    char buf = 0;  // Buffer to store the character read
    
    // termios is a structure that holds terminal settings
    // We use memset to properly initialize all fields to zero
    // This avoids compiler warnings about uninitialized members
    struct termios old;
    memset(&old, 0, sizeof(old));  // Initialize all bytes to 0
    
    // Flush any pending output before changing terminal settings
    fflush(stdout);
    
    // tcgetattr() gets the current terminal attributes
    // Parameters: file descriptor (0 = stdin), pointer to termios struct
    if (tcgetattr(0, &old) < 0) {
        perror("tcgetattr()");  // Print error message if it fails
    }
    
    // Modify the local flags (c_lflag):
    // ~ICANON: Disable canonical mode (don't wait for Enter)
    // ~ECHO: Disable echo (don't display typed characters)
    // The ~ operator is bitwise NOT, and &= clears those specific bits
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    
    // Set minimum number of characters to read and timeout
    old.c_cc[VMIN] = 1;   // Wait for at least 1 character
    old.c_cc[VTIME] = 0;  // No timeout
    
    // Apply the new settings immediately (TCSANOW)
    if (tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }
    
    // Read a single character from stdin (file descriptor 0)
    if (read(0, &buf, 1) < 0) {
        perror("read()");
    }
    
    // Restore the original terminal settings
    // Re-enable canonical mode and echo
    old.c_lflag |= ICANON;  // |= sets the bit (opposite of &= ~)
    old.c_lflag |= ECHO;
    
    // Apply restored settings (TCSADRAIN waits for output to finish)
    if (tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }
    
    return buf;
}

/*
 * getchWithTimeout() - Get a character with timeout for real-time updates (Sprint 6)
 * 
 * This allows the game to update the display (passive income) even while
 * waiting for user input. Uses select() to wait with a timeout.
 * 
 * Parameters:
 *   - timeoutMs: Maximum time to wait in milliseconds
 * 
 * Returns: The character pressed, or 0 if timeout occurred
 */
char getchWithTimeout(int timeoutMs) {
    struct termios old, current;
    memset(&old, 0, sizeof(old));
    
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    current.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &current);
    
    // Use select() to wait for input with timeout
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);  // 0 = stdin
    
    struct timeval timeout;
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;
    
    char result = 0;
    int ready = select(1, &readfds, NULL, NULL, &timeout);
    
    if (ready > 0 && FD_ISSET(0, &readfds)) {
        read(0, &result, 1);
    }
    
    // Restore terminal settings
    tcsetattr(0, TCSANOW, &old);
    
    return result;
}

/*
 * clearScreen() - Clear the terminal screen and move cursor to top-left
 * 
 * Uses ANSI escape codes:
 * - \033[2J: Clear entire screen
 * - \033[1;1H: Move cursor to row 1, column 1 (top-left)
 */
void clearScreen() {
    cout << "\033[2J\033[1;1H";
}

/*
 * printLine() - Print a horizontal line for visual separation
 * 
 * Parameters:
 *   - ch: The character to use for the line (default '=')
 *   - length: How many characters wide (default 50)
 *   - color: ANSI color code to use (default CYAN)
 * 
 * Default parameters in C++ allow calling the function with fewer arguments.
 * If not provided, the default values are used.
 */
void printLine(char ch = '=', int length = 50, const string& color = Color::CYAN) {
    cout << color;
    // Loop 'length' times, printing the character each time
    for (int i = 0; i < length; i++) {
        cout << ch;
    }
    cout << Color::RESET << endl;
}

/*
 * printCentered() - Print text centered within a given width
 * 
 * Parameters:
 *   - text: The string to center
 *   - width: Total width to center within (default 50)
 *   - color: ANSI color code (default empty = no color change)
 */
void printCentered(const string& text, int width = 50, const string& color = "") {
    // Calculate padding needed on the left side
    // (width - text.length()) gives total padding, divide by 2 for left side
    int padding = (width - text.length()) / 2;
    
    // Ensure padding is not negative (if text is longer than width)
    if (padding < 0) padding = 0;
    
    // Print spaces for padding, then the colored text
    cout << string(padding, ' ') << color << text << Color::RESET << endl;
}

/*
 * kbhit() - Check if a key has been pressed (non-blocking)
 * 
 * This function checks if there's input available on stdin without blocking.
 * It's essential for incremental games where we need to:
 * 1. Update passive income continuously
 * 2. Still respond to user input when it happens
 * 
 * Uses select() system call which can monitor file descriptors with a timeout.
 * We set timeout to 0 for immediate (non-blocking) check.
 * 
 * Returns: true if a key is available to read, false otherwise
 */
bool kbhit() {
    // timeval structure holds seconds and microseconds
    struct timeval tv;
    tv.tv_sec = 0;   // 0 seconds
    tv.tv_usec = 0;  // 0 microseconds = immediate return
    
    // fd_set is a set of file descriptors to monitor
    fd_set fds;
    FD_ZERO(&fds);        // Clear the set
    FD_SET(0, &fds);      // Add stdin (file descriptor 0) to the set
    
    // select() monitors file descriptors for activity
    // Parameters: highest fd + 1, read fds, write fds, error fds, timeout
    // Returns: number of ready descriptors, 0 if timeout, -1 if error
    return select(1, &fds, NULL, NULL, &tv) > 0;
}

/*
 * formatMoney() - Format a double as a currency string with appropriate suffix
 * 
 * For large numbers, uses suffixes like K (thousand), M (million), etc.
 * This is common in incremental games where numbers get very large.
 * 
 * Parameters:
 *   - amount: The money value to format
 * 
 * Returns: A formatted string like "$1.23K" or "$456.78M"
 */
string formatMoney(double amount) {
    // ostringstream allows us to build a string using << operators
    // This is useful for complex string formatting
    ostringstream oss;
    oss << fixed;  // Use fixed-point notation
    
    if (amount >= 1e12) {
        oss << setprecision(2) << (amount / 1e12) << "T";  // Trillion
    } else if (amount >= 1e9) {
        oss << setprecision(2) << (amount / 1e9) << "B";   // Billion
    } else if (amount >= 1e6) {
        oss << setprecision(2) << (amount / 1e6) << "M";   // Million
    } else if (amount >= 1e3) {
        oss << setprecision(2) << (amount / 1e3) << "K";   // Thousand
    } else {
        oss << setprecision(2) << amount;                   // Normal
    }
    
    return "$" + oss.str();
}

// ============================================================================
// GAMBLING GAME CLASS
// ============================================================================
/*
 * Classes in C++ are blueprints for creating objects. They encapsulate:
 * - Data (member variables) - the state of the object
 * - Functions (member methods) - the behavior of the object
 * 
 * Access specifiers:
 * - private: Only accessible within the class itself
 * - public: Accessible from anywhere
 * - protected: Accessible within class and derived classes
 */

class GamblingGame {
private:
    // ========================================================================
    // MEMBER VARIABLES (Private)
    // ========================================================================
    // These store the state of our game. They are private so they can only
    // be modified through our public methods, ensuring data integrity.
    
    double money;       // Player's current balance
    double betAmount;   // Current bet amount per game
    int round;          // Current round number
    int totalWins;      // Track total number of wins
    int totalLosses;    // Track total number of losses
    double peakMoney;   // Highest balance achieved (for statistics)
    
    // ========================================================================
    // INCREMENTAL GAME VARIABLES (Sprint 1)
    // ========================================================================
    /*
     * These variables support the passive income system - the core mechanic
     * that makes this an "incremental" or "idle" game.
     */
    
    vector<Generator> generators;  // All available passive income generators
    time_t lastUpdateTime;         // Last time we calculated passive income
    double lifetimeEarnings;       // Total money earned (for statistics/prestige)
    
    // ========================================================================
    // UPGRADE SYSTEM VARIABLES (Sprint 2)
    // ========================================================================
    /*
     * Upgrades modify game mechanics rather than producing passive income.
     * They improve win chances, payouts, and provide global bonuses.
     */
    
    vector<Upgrade> upgrades;      // All available upgrades
    
    // ========================================================================
    // SAVE/LOAD SYSTEM VARIABLES (Sprint 3)
    // ========================================================================
    /*
     * Persistence allows players to save their progress and continue later.
     * We also track when the game was last saved to calculate offline earnings.
     */
    
    static const string SAVE_FILE;  // File path for save data
    time_t lastSaveTime;            // When the game was last saved
    time_t gameStartTime;           // When this session started (for offline calc)
    
    // ========================================================================
    // PRESTIGE SYSTEM VARIABLES (Sprint 4)
    // ========================================================================
    /*
     * Prestige allows players to reset progress in exchange for permanent bonuses.
     * This extends gameplay by giving long-term goals.
     */
    
    int prestigeLevel;              // Number of times player has prestiged
    double prestigeMultiplier;      // Bonus multiplier from prestige (1.0 + 0.1 per level)
    double totalLifetimeEarnings;   // Earnings across all prestiges (for achievements)
    
    // ========================================================================
    // ANIMATION & UI VARIABLES (Sprint 7)
    // ========================================================================
    /*
     * Variables for animated UI elements and menu navigation.
     */
    
    int animationFrame;             // Frame counter for marquee animation
    int menuSelection;              // Currently selected menu item (0-indexed)
    static const int MENU_ITEMS = 13;  // Total number of menu options (added Specialization)
    
    // ========================================================================
    // ACHIEVEMENT SYSTEM VARIABLES (Sprint 4)
    // ========================================================================
    /*
     * Achievements reward players for reaching milestones.
     * Each achievement is tracked by a boolean flag.
     */
    
    bool achFirstWin;               // Win your first game
    bool achHighRoller;             // Win $1,000 in a single bet
    bool achMillionaire;            // Reach $1,000,000
    bool achLuckyStreak;            // Win 10 games in a row
    bool achGenerator10;            // Own 10 of any generator
    bool achAllUpgrades;            // Max out all upgrades
    bool achPrestige1;              // Prestige for the first time
    bool achPrestige5;              // Reach prestige level 5
    int currentWinStreak;           // Track consecutive wins for achievement
    
    // ========================================================================
    // PHASE B: ACHIEVEMENT REWARDS & PRESTIGE SHOP
    // ========================================================================
    /*
     * Achievement rewards give permanent bonuses for unlocking achievements.
     * Prestige stars are earned on prestige and spent in the prestige shop.
     */
    
    double achievementMultiplier;   // Income bonus from achievements (1.0 + 0.01 per achievement)
    int prestigeStars;              // Spendable currency earned from prestige
    
    // Prestige shop perk levels
    int perkStartingMoney;          // Start with more money after prestige
    int perkGeneratorDiscount;      // Generators cost less
    int perkOfflineCap;             // Offline earnings cap increase
    int perkPassiveBoost;           // Permanent passive income multiplier
    
    // ========================================================================
    // PHASE C: MILESTONES, SPECIALIZATION, SYNERGIES
    // ========================================================================
    /*
     * Milestones unlock new mechanics at progression thresholds.
     * Specialization gives strategic identity to playstyles.
     * Synergies add depth to generator combinations.
     */
    
    // Milestone unlocks
    bool milestoneUnlocked[10];     // Track which milestones are unlocked
    
    // Specialization system
    int specializationId;           // 0=none, 1=Wheel, 2=Slots, 3=Blackjack, 4=Roulette
    
    // Interest/bank system (unlocked by milestone)
    bool interestUnlocked;          // Whether interest system is available
    double interestRate;            // Interest rate per second
    
public:
    // ========================================================================
    // CONSTRUCTOR
    // ========================================================================
    /*
     * Constructor - Special method called when an object is created
     * 
     * The syntax after the colon is called an "initialization list".
     * It's more efficient than assigning values in the constructor body
     * because members are initialized directly rather than default-initialized
     * then assigned.
     * 
     * Format: ClassName() : member1(value1), member2(value2) { body }
     */
    GamblingGame() : money(100.0), betAmount(10.0), round(1), 
                     totalWins(0), totalLosses(0), peakMoney(100.0),
                     lifetimeEarnings(0.0), lastSaveTime(0), gameStartTime(0),
                     prestigeLevel(0), prestigeMultiplier(1.0), totalLifetimeEarnings(0.0),
                     animationFrame(0), menuSelection(0),
                     achFirstWin(false), achHighRoller(false), achMillionaire(false),
                     achLuckyStreak(false), achGenerator10(false), achAllUpgrades(false),
                     achPrestige1(false), achPrestige5(false), currentWinStreak(0),
                     achievementMultiplier(1.0), prestigeStars(0),
                     perkStartingMoney(0), perkGeneratorDiscount(0), perkOfflineCap(0), perkPassiveBoost(0),
                     specializationId(0), interestUnlocked(false), interestRate(0.001) {
        // Initialize milestones as locked
        for (int i = 0; i < 10; i++) {
            milestoneUnlocked[i] = false;
        }
        // srand() seeds the random number generator
        // time(0) returns current time in seconds since epoch
        // This ensures different random sequences each time we run
        srand(time(0));
        
        // Initialize the last update time to now
        lastUpdateTime = time(0);
        gameStartTime = time(0);
        
        // Initialize all generators
        // Each generator has: name, shortName, baseIncome, baseCost, costMultiplier
        initializeGenerators();
        
        // Initialize all upgrades (Sprint 2)
        initializeUpgrades();
    }
    
    /*
     * initializeGenerators() - Set up all passive income generators
     * 
     * This creates the progression of generators available in the game.
     * Each generator is more expensive but produces more income.
     * 
     * The vector::push_back() method adds an element to the end of the vector.
     * We're creating Generator objects inline and adding them to our vector.
     */
    void initializeGenerators() {
        // Early game generators (cheap, low income)
        // Format: name, shortName, description, icon, baseIncome, baseCost, costMultiplier
        generators.push_back(Generator(
            "Tip Jar", "TIP",
            "A humble jar collecting spare change from grateful winners",
            "🫙", 0.1, 10, 1.15));
        generators.push_back(Generator(
            "Penny Slots", "PNY",
            "Old-school one-armed bandits that never stop spinning",
            "🎰", 0.5, 50, 1.15));
        generators.push_back(Generator(
            "Auto-Wheel", "AWL",
            "A mechanical wheel that spins itself around the clock",
            "🎡", 2.0, 200, 1.18));
        
        // Mid game generators
        generators.push_back(Generator(
            "Card Shark", "CRD",
            "A skilled dealer who always finds an edge at the tables",
            "🃏", 5.0, 500, 1.20));
        generators.push_back(Generator(
            "VIP Table", "VIP",
            "An exclusive high-stakes table for the elite gamblers",
            "👑", 15.0, 1500, 1.22));
        generators.push_back(Generator(
            "High Roller", "HRL",
            "A wealthy patron who bets big and tips even bigger",
            "💎", 50.0, 5000, 1.25));
        
        // Late game generators
        generators.push_back(Generator(
            "Casino Floor", "FLR",
            "An entire floor of games generating constant revenue",
            "🏛️", 200.0, 20000, 1.28));
        generators.push_back(Generator(
            "Casino Empire", "EMP",
            "A chain of casinos spanning the globe, printing money 24/7",
            "🌍", 1000.0, 100000, 1.30));
    }
    
    /*
     * initializeUpgrades() - Set up all available upgrades (Sprint 2)
     * 
     * Each upgrade has:
     *   - name: Display name
     *   - description: What it does
     *   - type: UpgradeType enum value
     *   - maxLevel: Maximum upgrade level
     *   - baseCost: Cost for first level
     *   - costMultiplier: How much cost increases per level
     *   - baseValue: Starting value (before any upgrades)
     *   - valuePerLevel: How much value improves per level
     */
    void initializeUpgrades() {
        // ═══════════════════════════════════════════════════════════════════
        // WHEEL SPIN UPGRADES
        // Base: 50% chance, 2x payout
        // ═══════════════════════════════════════════════════════════════════
        upgrades.push_back(Upgrade(
            "Lucky Wheel",
            "Grease the wheel bearings for better odds. Each level adds +2% win chance.",
            UpgradeType::WHEEL_CHANCE,
            5, 100, 2.5, 50.0, 2.0
        ));
        
        upgrades.push_back(Upgrade(
            "Wheel Jackpot",
            "Upgrade the prize pool! Each level adds +0.25x to your payout multiplier.",
            UpgradeType::WHEEL_PAYOUT,
            4, 200, 3.0, 2.0, 0.25
        ));
        
        // ═══════════════════════════════════════════════════════════════════
        // SLOTS UPGRADES
        // Base: ~16.7% chance (1/6), 5x payout
        // ═══════════════════════════════════════════════════════════════════
        upgrades.push_back(Upgrade(
            "Loaded Reels",
            "Weight the slot reels in your favor. Each level adds +3% match chance.",
            UpgradeType::SLOTS_CHANCE,
            5, 300, 2.5, 16.67, 3.0
        ));
        
        upgrades.push_back(Upgrade(
            "Slots Bonanza",
            "Bigger jackpots await! Each level adds +1x to your slots payout.",
            UpgradeType::SLOTS_PAYOUT,
            5, 500, 3.0, 5.0, 1.0
        ));
        
        // ═══════════════════════════════════════════════════════════════════
        // BLACKJACK UPGRADES (Sprint 5)
        // Base: 2x payout, dealer follows standard rules
        // ═══════════════════════════════════════════════════════════════════
        upgrades.push_back(Upgrade(
            "Card Counter",
            "Learn to count cards like a pro. Each level adds +0.2x to blackjack payouts.",
            UpgradeType::BLACKJACK_PAYOUT,
            5, 400, 2.5, 2.0, 0.2
        ));
        
        upgrades.push_back(Upgrade(
            "Dealer Jinx",
            "Bad luck follows the dealer. Each level adds +3% chance the dealer busts.",
            UpgradeType::DEALER_BUST,
            5, 600, 3.0, 0.0, 3.0
        ));
        
        // ═══════════════════════════════════════════════════════════════════
        // ROULETTE UPGRADES (Sprint 5)
        // Base: 2x color payout, 35x number payout
        // ═══════════════════════════════════════════════════════════════════
        upgrades.push_back(Upgrade(
            "Croupier's Friend",
            "The croupier owes you a favor. Each level adds +0.5x to roulette payouts.",
            UpgradeType::ROULETTE_PAYOUT,
            4, 350, 2.8, 2.0, 0.5
        ));
        
        upgrades.push_back(Upgrade(
            "Lucky 7s",
            "Seven is your lucky number! Each level adds +5x bonus when hitting 7, 17, or 27.",
            UpgradeType::LUCKY_NUMBER,
            5, 800, 3.2, 0.0, 5.0
        ));
        
        // ═══════════════════════════════════════════════════════════════════
        // GLOBAL UPGRADES
        // Affect all games and income
        // ═══════════════════════════════════════════════════════════════════
        upgrades.push_back(Upgrade(
            "Income Boost",
            "Optimize your casino operations. Each level adds +10% to all passive income.",
            UpgradeType::INCOME_BOOST,
            10, 250, 2.0, 100.0, 10.0
        ));
        
        upgrades.push_back(Upgrade(
            "Lucky Charm",
            "Carry a rabbit's foot for luck. Each level adds +5% chance for 2x bonus on wins.",
            UpgradeType::LUCKY_BONUS,
            5, 1000, 3.5, 0.0, 5.0
        ));
    }
    
    // ========================================================================
    // UPGRADE HELPER METHODS
    // ========================================================================
    
    /*
     * getUpgradeValue() - Get the current value of a specific upgrade type
     * 
     * This method searches through all upgrades to find one matching the
     * given type and returns its current value.
     * 
     * Parameters:
     *   - type: The UpgradeType to look for
     * 
     * Returns: The current value of that upgrade, or a default if not found
     */
    double getUpgradeValue(UpgradeType type) const {
        for (const auto& upg : upgrades) {
            if (upg.getType() == type) {
                return upg.getCurrentValue();
            }
        }
        // Return sensible defaults if upgrade not found
        switch (type) {
            case UpgradeType::WHEEL_CHANCE: return 50.0;
            case UpgradeType::WHEEL_PAYOUT: return 2.0;
            case UpgradeType::SLOTS_CHANCE: return 16.67;
            case UpgradeType::SLOTS_PAYOUT: return 5.0;
            case UpgradeType::INCOME_BOOST: return 100.0;
            case UpgradeType::LUCKY_BONUS: return 0.0;
            default: return 0.0;
        }
    }
    
    /*
     * getIncomeMultiplier() - Calculate the income boost multiplier
     * 
     * Converts the INCOME_BOOST percentage to a multiplier.
     * 100% = 1.0x, 150% = 1.5x, 200% = 2.0x, etc.
     */
    double getIncomeMultiplier() const {
        return getUpgradeValue(UpgradeType::INCOME_BOOST) / 100.0;
    }
    
    // ========================================================================
    // DISPLAY METHODS
    // ========================================================================
    
    /*
     * displayHeader() - Show the game title with animated marquee (Sprint 7)
     * 
     * The stars around the title blink like a casino marquee sign.
     * Animation is based on animationFrame counter that increments each refresh.
     */
    void displayHeader() {
        // Marquee pattern - stars blink in sequence using escape codes
        string star1 = (animationFrame % 4 == 0) ? UI::BLACK_STAR : UI::WHITE_STAR;
        string star2 = (animationFrame % 4 == 1) ? UI::BLACK_STAR : UI::WHITE_STAR;
        string star3 = (animationFrame % 4 == 2) ? UI::BLACK_STAR : UI::WHITE_STAR;
        string star4 = (animationFrame % 4 == 3) ? UI::BLACK_STAR : UI::WHITE_STAR;
        
        // Color cycling for extra flair
        string colors[] = {Color::YELLOW, Color::RED, Color::GREEN, Color::CYAN};
        string starColor = colors[animationFrame % 4];
        
        cout << Color::YELLOW;
        cout << "  " << UI::TL_CORNER;
        for (int i = 0; i < 46; i++) cout << UI::H_LINE;
        cout << UI::TR_CORNER << "\n";
        
        // Top star row
        cout << "  " << UI::V_LINE << " " << starColor << star1 << " " << star2 << " " << star3 << " " << star4;
        cout << Color::YELLOW << "                              ";
        cout << starColor << star4 << " " << star3 << " " << star2 << " " << star1 << Color::YELLOW << " " << UI::V_LINE << "\n";
        
        // Title - using escape codes for slot machine emoji
        cout << "  " << UI::V_LINE << Color::BOLD << Color::YELLOW;
        cout << "        " << UI::SLOT << " LUCKY SPIN CASINO " << UI::SLOT << "        ";
        cout << Color::RESET << Color::YELLOW << UI::V_LINE << "\n";
        
        // Bottom star row
        cout << "  " << UI::V_LINE << " " << starColor << star4 << " " << star3 << " " << star2 << " " << star1;
        cout << Color::YELLOW << "                              ";
        cout << starColor << star1 << " " << star2 << " " << star3 << " " << star4 << Color::YELLOW << " " << UI::V_LINE << "\n";
        
        cout << "  " << UI::BL_CORNER;
        for (int i = 0; i < 46; i++) cout << UI::H_LINE;
        cout << UI::BR_CORNER << Color::RESET << "\n";
        
        // Increment animation frame
        animationFrame++;
    }

    /*
     * getTotalPassiveIncome() - Calculate total passive income per second
     * 
     * Iterates through all generators and sums up their income.
     * Then applies the Income Boost upgrade multiplier (Sprint 2).
     * 
     * Returns: Total income per second from all generators (with multiplier)
     */
    /*
     * getAchievementCount() - Count unlocked achievements
     */
    int getAchievementCount() const {
        int count = 0;
        if (achFirstWin) count++;
        if (achHighRoller) count++;
        if (achMillionaire) count++;
        if (achLuckyStreak) count++;
        if (achGenerator10) count++;
        if (achAllUpgrades) count++;
        if (achPrestige1) count++;
        if (achPrestige5) count++;
        return count;
    }
    
    /*
     * getPassiveBoostFromPerks() - Calculate passive income multiplier from prestige perks
     */
    double getPassiveBoostFromPerks() const {
        return 1.0 + (perkPassiveBoost * 0.05);  // +5% per level
    }
    
    /*
     * getTotalGeneratorsOwned() - Count all generators owned (Phase C)
     */
    int getTotalGeneratorsOwned() const {
        int total = 0;
        for (const auto& gen : generators) {
            total += gen.getOwned();
        }
        return total;
    }
    
    /*
     * getGeneratorSynergyMultiplier() - Calculate synergy bonuses (Phase C)
     * 
     * Higher-tier generators boost lower tiers.
     * Threshold bonuses at 10/25/50/100 owned.
     */
    double getGeneratorSynergyMultiplier(int genIndex) const {
        double multiplier = 1.0;
        
        if (genIndex < 0 || genIndex >= (int)generators.size()) return multiplier;
        
        const Generator& gen = generators[genIndex];
        int owned = gen.getOwned();
        
        // Threshold bonuses for this generator
        if (owned >= 10) multiplier *= 1.1;   // +10% at 10 owned
        if (owned >= 25) multiplier *= 1.15;  // +15% more at 25
        if (owned >= 50) multiplier *= 1.2;   // +20% more at 50
        if (owned >= 100) multiplier *= 1.25; // +25% more at 100
        
        // Higher-tier generators boost this one (1% per owned)
        for (int i = genIndex + 1; i < (int)generators.size(); i++) {
            int higherOwned = generators[i].getOwned();
            multiplier *= (1.0 + higherOwned * 0.01);
        }
        
        return multiplier;
    }
    
    /*
     * getSpecializationMultiplier() - Get bonus from specialization (Phase C)
     */
    double getSpecializationMultiplier() const {
        if (specializationId == 0) return 1.0;
        return 1.5;  // +50% to specialized game type
    }
    
    double getTotalPassiveIncome() const {
        double total = 0.0;
        
        // Sum up generator income with synergy bonuses (Phase C)
        for (size_t i = 0; i < generators.size(); i++) {
            double baseIncome = generators[i].getIncomePerSecond();
            double synergyMult = getGeneratorSynergyMultiplier(i);
            total += baseIncome * synergyMult;
        }
        
        // Apply Income Boost upgrade multiplier (Sprint 2)
        // getIncomeMultiplier() returns 1.0 at base, up to 2.0 at max level
        total *= getIncomeMultiplier();
        
        // Apply Prestige multiplier (Sprint 4)
        // Each prestige level adds +10% to all income
        total *= prestigeMultiplier;
        
        // Apply Achievement multiplier (Phase B)
        // Each achievement adds +1% to all income
        total *= achievementMultiplier;
        
        // Apply Prestige perk passive boost (Phase B)
        total *= getPassiveBoostFromPerks();
        
        return total;
    }
    
    /*
     * checkMilestones() - Check and unlock milestones (Phase C)
     */
    void checkMilestones() {
        // Milestone 0: Own 10 total generators
        if (!milestoneUnlocked[0] && getTotalGeneratorsOwned() >= 10) {
            milestoneUnlocked[0] = true;
            showMilestone("Generator Collector", "Own 10 generators total");
        }
        
        // Milestone 1: Reach $10k passive income/sec
        if (!milestoneUnlocked[1] && getTotalPassiveIncome() >= 10.0) {
            milestoneUnlocked[1] = true;
            showMilestone("Income Milestone", "Reach $10/sec passive income");
        }
        
        // Milestone 2: First prestige - unlock interest system
        if (!milestoneUnlocked[2] && prestigeLevel >= 1) {
            milestoneUnlocked[2] = true;
            interestUnlocked = true;
            showMilestone("Banking Unlocked", "Interest system now available!");
        }
        
        // Milestone 3: Own 50 total generators
        if (!milestoneUnlocked[3] && getTotalGeneratorsOwned() >= 50) {
            milestoneUnlocked[3] = true;
            showMilestone("Generator Hoarder", "Own 50 generators total");
        }
        
        // Milestone 4: Reach $1M total lifetime earnings
        if (!milestoneUnlocked[4] && totalLifetimeEarnings >= 1000000.0) {
            milestoneUnlocked[4] = true;
            showMilestone("Millionaire Club", "$1M total lifetime earnings");
        }
        
        // Milestone 5: Reach prestige level 3
        if (!milestoneUnlocked[5] && prestigeLevel >= 3) {
            milestoneUnlocked[5] = true;
            showMilestone("Prestige Veteran", "Reach prestige level 3");
        }
    }
    
    /*
     * showMilestone() - Display milestone unlock notification (Phase C)
     */
    void showMilestone(const string& name, const string& desc) {
        cout << "\n";
        cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD 
             << " 🏆 MILESTONE UNLOCKED! 🏆 " << Color::RESET << "\n";
        cout << "  " << Color::YELLOW << name << Color::RESET << "\n";
        cout << "  " << Color::DIM << desc << Color::RESET << "\n";
        usleep(1500000);
    }
    
    /*
     * updatePassiveIncome() - Apply passive income based on elapsed time
     * 
     * This is the heart of the incremental game mechanic. It calculates
     * how much time has passed since the last update and adds the appropriate
     * amount of money based on passive income rate.
     * 
     * Called frequently in the game loop to keep income flowing.
     */
    void updatePassiveIncome() {
        // Get current time
        time_t now = time(0);
        
        // Calculate seconds elapsed since last update
        // difftime() returns the difference between two time_t values in seconds
        double deltaSeconds = difftime(now, lastUpdateTime);
        
        // Only update if at least some time has passed
        if (deltaSeconds > 0) {
            // Calculate income earned during this period
            double income = getTotalPassiveIncome() * deltaSeconds;
            
            // Add interest if unlocked (Phase C)
            if (interestUnlocked && money > 0) {
                double interest = money * interestRate * deltaSeconds;
                income += interest;
            }
            
            // Add to balance and lifetime earnings
            money += income;
            lifetimeEarnings += income;
            
            // Update peak money if we've reached a new high
            updatePeakMoney();
            
            // Reset the timer
            lastUpdateTime = now;
            
            // Check for milestone unlocks (Phase C)
            checkMilestones();
        }
    }
    
    /*
     * displayStats() - Show current game statistics with Unicode decorations
     * 
     * Updated in Sprint 6 to use Unicode icons and show owned generators.
     */
    void displayStats() {
        cout << "\n";
        cout << "  " << UI::TL_CORNER;
        for (int i = 0; i < 46; i++) cout << UI::H_LINE;
        cout << UI::TR_CORNER << "\n";
        
        // Display balance with icon
        cout << "  " << UI::V_LINE << " " << UI::MONEY << " Balance: ";
        if (money >= 100) {
            cout << Color::GREEN;
        } else if (money >= 50) {
            cout << Color::YELLOW;
        } else {
            cout << Color::RED;
        }
        cout << formatMoney(money) << Color::RESET;
        
        // Display current bet
        cout << "  " << UI::DICE << " Bet: " << Color::CYAN 
             << "$" << fixed << setprecision(2) << betAmount << Color::RESET;
        
        // Display round number
        cout << "  " << UI::CHART << " Rnd: " << Color::MAGENTA 
             << round << Color::RESET;
        cout << " " << UI::V_LINE << "\n";
        
        // Display passive income with fire icon if high
        double passiveIncome = getTotalPassiveIncome();
        cout << "  " << UI::V_LINE << " ";
        if (passiveIncome >= 10) {
            cout << UI::FIRE;
        } else {
            cout << UI::CLOCK;
        }
        cout << " Income: " << Color::BOLD << Color::GREEN;
        if (passiveIncome > 0) {
            cout << "+$" << fixed << setprecision(2) << passiveIncome << "/sec";
        } else {
            cout << Color::DIM << "$0.00/sec";
        }
        cout << Color::RESET;
        
        // Show prestige level if > 0
        if (prestigeLevel > 0) {
            cout << "  " << UI::STAR << " P" << prestigeLevel << " (" 
                 << fixed << setprecision(1) << prestigeMultiplier << "x)";
        }
        cout << " " << UI::V_LINE << "\n";
        
        // Win/Loss statistics with icons
        cout << "  " << UI::V_LINE << " " << Color::GREEN << UI::CHECK << " Wins: " << totalWins << Color::RESET
             << "  " << Color::RED << UI::CROSS_X << " Losses: " << totalLosses << Color::RESET
             << "  " << UI::TROPHY << " Peak: " << Color::YELLOW << formatMoney(peakMoney) 
             << Color::RESET << " " << UI::V_LINE << "\n";
        
        // Show owned generators as icons
        cout << "  " << UI::V_LINE << " Generators: ";
        bool hasGenerators = false;
        for (const auto& gen : generators) {
            if (gen.getOwned() > 0) {
                hasGenerators = true;
                cout << gen.getIcon();
                if (gen.getOwned() > 1) {
                    cout << "x" << gen.getOwned() << " ";
                } else {
                    cout << " ";
                }
            }
        }
        if (!hasGenerators) {
            cout << Color::DIM << "(none - visit shop!)" << Color::RESET;
        }
        cout << UI::V_LINE << "\n";
        
        cout << "  " << UI::BL_CORNER;
        for (int i = 0; i < 46; i++) cout << UI::H_LINE;
        cout << UI::BR_CORNER << "\n";
    }
    
    /*
     * displayMenu() - Show the main menu with arrow/WASD navigation (Sprint 7)
     * 
     * Menu items are highlighted based on menuSelection.
     * Use W/S or Up/Down arrows to navigate, Enter/Space to select.
     */
    void displayMenu() {
        // Get current upgrade values to show in menu
        double wheelChance = getUpgradeValue(UpgradeType::WHEEL_CHANCE);
        double wheelPayout = getUpgradeValue(UpgradeType::WHEEL_PAYOUT);
        double slotsChance = getUpgradeValue(UpgradeType::SLOTS_CHANCE);
        double slotsPayout = getUpgradeValue(UpgradeType::SLOTS_PAYOUT);
        double bjPayout = getUpgradeValue(UpgradeType::BLACKJACK_PAYOUT);
        (void)bjPayout;  // Used in menu item description
        
        // Menu item definitions with icons
        struct MenuItem {
            string icon;
            string name;
            string desc;
        };
        
        string specName = specializationId == 0 ? "none" : 
                         (specializationId == 1 ? "Wheel" : 
                         (specializationId == 2 ? "Slots" : 
                         (specializationId == 3 ? "Blackjack" : "Roulette")));
        
        MenuItem items[] = {
            {"\xF0\x9F\x8E\xA1", "Spin the Wheel", "(" + to_string((int)wheelChance) + "% chance, " + to_string(wheelPayout).substr(0,4) + "x)"},
            {"\xF0\x9F\x8E\xB0", "Play Slots", "(" + to_string((int)slotsChance) + "% chance, " + to_string(slotsPayout).substr(0,3) + "x)"},
            {"\xF0\x9F\x83\x8F", "Blackjack", "(payout: " + to_string(bjPayout).substr(0,3) + "x)"},
            {"\xF0\x9F\x8E\xB2", "Roulette", "(up to 35x payout)"},
            {"\xF0\x9F\x8F\xAA", "Generator Shop", "(passive income)"},
            {"\xE2\xAC\x86", "Upgrade Shop", "(improve odds)"},
            {"\xF0\x9F\x92\xB5", "Change Bet", "(current: $" + to_string((int)betAmount) + ")"},
            {"\xE2\xAD\x90", "Prestige", "(level " + to_string(prestigeLevel) + ")"},
            {"\xF0\x9F\x8E\xAF", "Specialization", "(" + specName + ")"},
            {"\xF0\x9F\x8F\x86", "Achievements", ""},
            {"\xF0\x9F\x92\xBE", "Save Game", ""},
            {"\xF0\x9F\x94\x84", "Restart Game", "(reset all progress)"},
            {"\xF0\x9F\x9A\xAA", "Quit Game", ""}
        };
        
        cout << "\n";
        cout << "  " << Color::DIM << "Use " << Color::RESET << Color::CYAN << "W/S" 
             << Color::RESET << Color::DIM << " or " << Color::RESET << Color::CYAN 
             << "↑/↓" << Color::RESET << Color::DIM << " to navigate, " 
             << Color::RESET << Color::CYAN << "Enter" << Color::RESET 
             << Color::DIM << " to select" << Color::RESET << "\n\n";
        
        // Display menu items with cursor
        for (int i = 0; i < MENU_ITEMS; i++) {
            // Add section headers
            if (i == 0) cout << "  " << Color::BOLD << "GAMES" << Color::RESET << "\n";
            if (i == 4) cout << "\n  " << Color::BOLD << "SHOP" << Color::RESET << "\n";
            if (i == 6) cout << "\n  " << Color::BOLD << "OPTIONS" << Color::RESET << "\n";
            
            // Highlight selected item
            if (i == menuSelection) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << items[i].icon << " " << items[i].name;
                // Pad to consistent width
                int padding = 20 - items[i].name.length();
                for (int p = 0; p < padding; p++) cout << " ";
                cout << Color::RESET << " " << Color::CYAN << items[i].desc << Color::RESET << "\n";
            } else {
                cout << "    " << items[i].icon << " " << Color::WHITE << items[i].name << Color::RESET;
                int padding = 20 - items[i].name.length();
                for (int p = 0; p < padding; p++) cout << " ";
                cout << " " << Color::DIM << items[i].desc << Color::RESET << "\n";
            }
        }
    }
    
    /*
     * displayStatus() - Main display function combining all elements
     */
    void displayStatus() {
        clearScreen();
        displayHeader();
        displayStats();
        displayMenu();
    }
    
    // ========================================================================
    // GAME LOGIC METHODS
    // ========================================================================
    
    /*
     * updatePeakMoney() - Track the highest balance achieved
     * 
     * This is a simple helper method that updates peakMoney if current
     * balance exceeds it. Good practice to break logic into small functions.
     */
    void updatePeakMoney() {
        if (money > peakMoney) {
            peakMoney = money;
        }
    }
    
    /*
     * changeBet() - Allow player to modify their bet amount
     * 
     * Uses arrow/WASD navigation with preset bet amounts and custom option.
     * Demonstrates input validation - we check that the new bet is:
     * 1. Greater than 0 (can't bet nothing or negative)
     * 2. Less than or equal to current balance (can't bet more than you have)
     */
    void changeBet() {
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
     * canAffordBet() - Check if player has enough money for current bet
     * 
     * Returns: true if player can afford the bet, false otherwise
     * 
     * This is a "guard" function - it prevents invalid game states
     */
    bool canAffordBet() {
        if (money < betAmount) {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money for that bet!" 
                 << Color::RESET << "\n";
            cout << "  Your balance: $" << fixed << setprecision(2) << money << "\n";
            cout << "  Current bet: $" << fixed << setprecision(2) << betAmount << "\n";
            cout << "\n  Press any key to continue...";
            getch();
            return false;
        }
        return true;
    }
    
    /*
     * applyLuckyBonus() - Check and apply lucky bonus multiplier (Sprint 2)
     * 
     * The Lucky Charm upgrade gives a chance for wins to be doubled.
     * 
     * Parameters:
     *   - amount: The base win amount
     * 
     * Returns: The amount (possibly doubled if lucky bonus triggers)
     */
    double applyLuckyBonus(double amount) {
        double luckyChance = getUpgradeValue(UpgradeType::LUCKY_BONUS);
        
        // Check if lucky bonus triggers (luckyChance is a percentage)
        if (luckyChance > 0 && (rand() % 100) < luckyChance) {
            cout << "  " << Color::BG_YELLOW << Color::BOLD << Color::RED 
                 << " LUCKY BONUS! 2x " << Color::RESET << "\n";
            return amount * 2.0;
        }
        return amount;
    }
    
    /*
     * spinWheel() - The wheel spinning game
     * 
     * Rules (affected by upgrades):
     * - Win chance: 50% base + WHEEL_CHANCE upgrade
     * - Payout: 2x base + WHEEL_PAYOUT upgrade
     * - Lucky Charm can double winnings
     */
    void spinWheel() {
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
        
        // Generate random outcome using upgraded win chance
        // rand() % 100 gives 0-99, compare against winChance percentage
        bool won = (rand() % 100) < winChance;
        
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
    string getColoredSymbol(int index) {
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
    void playSlots() {
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
                slots[i] = rand() % Symbols::COUNT;
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
        bool won = (rand() % 100) < winChance;
        
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
    void playBlackjack() {
        if (!canAffordBet()) return;
        
        money -= betAmount;
        
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('-', 50, Color::GREEN);
        printCentered("BLACKJACK", 50, Color::GREEN + Color::BOLD);
        printLine('-', 50, Color::GREEN);
        
        // Card values: 1-10 (Ace=1 or 11, face cards=10)
        // Simple representation: generate random values
        auto drawCard = []() -> int {
            int card = (rand() % 13) + 1;  // 1-13
            if (card > 10) card = 10;       // Face cards = 10
            return card;
        };
        
        auto getCardName = [](int card) -> string {
            if (card == 1) return "A";
            if (card == 10) return "10";
            if (card == 11) return "J";
            if (card == 12) return "Q";
            if (card == 13) return "K";
            return to_string(card);
        };
        
        // Calculate hand value, handling Aces
        auto calculateHand = [](vector<int>& hand) -> int {
            int total = 0;
            int aces = 0;
            for (int card : hand) {
                if (card == 1) {
                    aces++;
                    total += 11;  // Count Ace as 11 initially
                } else {
                    total += card;
                }
            }
            // Convert Aces from 11 to 1 if busting
            while (total > 21 && aces > 0) {
                total -= 10;
                aces--;
            }
            return total;
        };
        
        // Deal initial cards
        vector<int> playerHand, dealerHand;
        playerHand.push_back(drawCard());
        dealerHand.push_back(drawCard());
        playerHand.push_back(drawCard());
        dealerHand.push_back(drawCard());
        
        bool playerBust = false;
        bool playerStand = false;
        
        // Helper to get card suit (Unicode - Sprint 6)
        auto getCardSuit = [](int cardIndex) -> string {
            const string suits[] = {Cards::SPADE, Cards::HEART, Cards::DIAMOND, Cards::CLUB};
            return suits[cardIndex % 4];
        };
        
        // Helper to display a card with suit
        auto displayCard = [&](int card, int index) -> string {
            string suit = getCardSuit(index);
            string suitColor = (index % 4 == 1 || index % 4 == 2) ? Color::RED : Color::WHITE;
            return "[" + suitColor + getCardName(card) + suit + Color::RESET + "]";
        };
        
        int cardIndex = 0;  // Track card index for suits
        
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
            
            int playerTotal = calculateHand(playerHand);
            
            // Display current state with card suits
            cout << "\n  " << Color::WHITE << "Dealer shows: " << Color::CYAN 
                 << displayCard(dealerHand[0], 0) << " [" << Color::DIM << "??" << Color::RESET << "]" 
                 << Color::RESET << "\n";
            
            cout << "  " << Color::WHITE << "Your hand:    " << Color::YELLOW;
            for (size_t i = 0; i < playerHand.size(); i++) {
                cout << displayCard(playerHand[i], i + 2) << " ";
            }
            cout << Color::RESET << "= " << Color::BOLD << playerTotal << Color::RESET << "\n";
            
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
                    int newCard = drawCard();
                    playerHand.push_back(newCard);
                    cardIndex++;
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
                int newCard = drawCard();
                playerHand.push_back(newCard);
                cardIndex++;
            } else if (choice == 'q' || choice == 'Q') {
                // Quick key for Quit
                cout << "\n  " << Color::YELLOW << "You left the table. Bet forfeited." << Color::RESET << "\n";
                totalLosses++;
                cout << "\n  Press any key to continue...";
                getch();
                return;
            }
        }
        
        int playerTotal = calculateHand(playerHand);
        
        if (playerBust) {
            cout << "\n  " << Color::BG_RED << Color::WHITE << Color::BOLD 
                 << " BUST! Over 21 " << Color::RESET << "\n";
            cout << "  " << Color::RED << "-$" << fixed << setprecision(2) 
                 << betAmount << Color::RESET << "\n";
            totalLosses++;
        } else {
            // Dealer's turn
            cout << "\n  " << Color::WHITE << "Dealer reveals: " << Color::CYAN;
            for (int card : dealerHand) {
                cout << "[" << getCardName(card) << "] ";
            }
            int dealerTotal = calculateHand(dealerHand);
            cout << Color::RESET << "= " << dealerTotal << "\n";
            usleep(800000);
            
            // Dealer draws until 17+
            // Apply Dealer Jinx upgrade - chance for dealer to bust early
            double dealerBustChance = getUpgradeValue(UpgradeType::DEALER_BUST);
            bool forcedBust = (rand() % 100) < dealerBustChance;
            
            while (dealerTotal < 17) {
                int newCard = drawCard();
                dealerHand.push_back(newCard);
                dealerTotal = calculateHand(dealerHand);
                cout << "  Dealer draws: " << Color::CYAN << "[" << getCardName(newCard) 
                     << "]" << Color::RESET << " = " << dealerTotal << "\n";
                usleep(800000);
            }
            
            // If Dealer Jinx triggered and dealer didn't naturally bust, force a bust
            if (forcedBust && dealerTotal <= 21) {
                int bustCard = 10;  // Force a 10 to likely bust
                dealerHand.push_back(bustCard);
                dealerTotal = calculateHand(dealerHand);
                cout << "  " << Color::MAGENTA << "★ Dealer Jinx! ★" << Color::RESET << "\n";
                cout << "  Dealer draws: " << Color::CYAN << "[10]" 
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
                double payout = (playerTotal == 21 && playerHand.size() == 2) ? basePayout + 0.5 : basePayout;
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
    void playRoulette() {
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
        
        // Generate result (0-36)
        int result = rand() % 37;
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
    
    // ========================================================================
    // UPGRADE SHOP (Sprint 2 - Game Progression)
    // ========================================================================
    
    /*
     * displayUpgradeShop() - Show the upgrade purchase menu
     * 
     * Upgrades improve game mechanics like win chances and payouts.
     * Unlike generators, upgrades have a maximum level.
     * 
     * The shop displays:
     * - Upgrade name and current level
     * - Cost for next level
     * - Current and next values
     */
    void displayUpgradeShop() {
        bool inShop = true;
        int shopSelection = 0;  // 0 = back, 1+ = upgrades
        int maxSelection = upgrades.size();
        
        while (inShop) {
            updatePassiveIncome();
            
            clearScreen();
            printLine('=', 50, Color::BLUE);
            cout << Color::BOLD << Color::BLUE;
            printCentered("UPGRADE SHOP", 50);
            cout << Color::RESET;
            printLine('=', 50, Color::BLUE);
            
            cout << "\n  " << Color::WHITE << "Your Balance: " << Color::GREEN 
                 << formatMoney(money) << Color::RESET << "\n";
            cout << "  " << Color::DIM << "Use W/S to navigate, Enter to buy, Q to exit" << Color::RESET << "\n\n";
            
            printLine('-', 50, Color::DIM);
            
            // List all upgrades with cursor
            for (size_t i = 0; i < upgrades.size(); i++) {
                Upgrade& upg = upgrades[i];
                double cost = upg.getCurrentCost();
                bool maxed = upg.isMaxed();
                
                // Highlight selected item
                if ((int)i + 1 == shopSelection) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                    cout << upg.getName() << Color::RESET;
                } else {
                    cout << "    " << Color::WHITE << upg.getName() << Color::RESET;
                }
                
                cout << " Lv." << upg.getCurrentLevel() << "/" << upg.getMaxLevel();
                
                if (maxed) {
                    cout << Color::YELLOW << " [MAX]" << Color::RESET << "\n";
                } else {
                    string costColor = (money >= cost) ? Color::GREEN : Color::RED;
                    cout << "  " << costColor << formatMoney(cost) << Color::RESET << "\n";
                }
                
                cout << "      " << Color::DIM << upg.getDescription();
                cout << " (" << fixed << setprecision(1) << upg.getCurrentValue();
                if (!maxed) {
                    cout << " -> " << Color::GREEN << upg.getNextValue() << Color::RESET << Color::DIM;
                }
                cout << ")" << Color::RESET << "\n";
            }
            
            printLine('-', 50, Color::DIM);
            
            // Back option
            if (shopSelection == 0) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << "Back to Main Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::WHITE << "Back to Main Menu" << Color::RESET << "\n";
            }
            
            char choice = getchWithTimeout(1000);
            if (choice == 0) continue;
            
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
                shopSelection = (shopSelection - 1 + maxSelection + 1) % (maxSelection + 1);
            } else if (choice == 's' || choice == 'S') {
                shopSelection = (shopSelection + 1) % (maxSelection + 1);
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (shopSelection == 0) {
                    inShop = false;
                } else {
                    purchaseUpgrade(shopSelection - 1);
                }
            } else if (choice == 'q' || choice == 'Q' || choice == 'a' || choice == 'A') {
                inShop = false;
            } else if (choice >= '1' && choice <= '0' + (int)upgrades.size()) {
                purchaseUpgrade(choice - '1');
            } else if (choice == '0') {
                inShop = false;
            }
        }
    }
    
    /*
     * purchaseUpgrade() - Attempt to buy an upgrade level
     * 
     * Parameters:
     *   - index: Index of the upgrade in the upgrades vector
     */
    void purchaseUpgrade(int index) {
        if (index < 0 || index >= (int)upgrades.size()) {
            return;
        }
        
        Upgrade& upg = upgrades[index];
        
        if (upg.isMaxed()) {
            cout << "\n  " << Color::YELLOW << "Already at max level!" << Color::RESET;
            usleep(800000);
            return;
        }
        
        double cost = upg.getCurrentCost();
        
        if (money >= cost) {
            money -= cost;
            upg.purchase();
            
            cout << "\n  " << Color::GREEN << UI::CHECK << " Upgraded " << upg.getName() 
                 << " to Lv." << upg.getCurrentLevel() << "!" << Color::RESET;
            cout << "\n  " << Color::DIM << "New value: " << fixed << setprecision(1) 
                 << upg.getCurrentValue() << Color::RESET;
            
            usleep(800000);
        } else {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need " 
                 << formatMoney(cost) << Color::RESET;
            usleep(800000);
        }
    }
    
    // ========================================================================
    // SAVE/LOAD SYSTEM (Sprint 3 - Persistence)
    // ========================================================================
    
    /*
     * saveGame() - Save the current game state to a file
     * 
     * File format is a simple key=value text format, one per line.
     * This is easy to read, debug, and parse.
     * 
     * We save:
     * - Version number (for future compatibility)
     * - All player stats (money, wins, losses, etc.)
     * - Generator owned counts
     * - Upgrade levels
     * - Timestamp (for offline earnings calculation)
     * 
     * Returns: true if save was successful, false otherwise
     */
    bool saveGame() {
        // ofstream is "output file stream" - used for writing to files
        ofstream file(SAVE_FILE);
        
        if (!file.is_open()) {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to save game!" << Color::RESET;
            return false;
        }
        
        // Write save file header with version
        file << "LUCKY_SPIN_SAVE_V1" << endl;
        file << "timestamp=" << time(0) << endl;
        
        // Save player stats
        file << "money=" << fixed << setprecision(2) << money << endl;
        file << "betAmount=" << fixed << setprecision(2) << betAmount << endl;
        file << "round=" << round << endl;
        file << "totalWins=" << totalWins << endl;
        file << "totalLosses=" << totalLosses << endl;
        file << "peakMoney=" << fixed << setprecision(2) << peakMoney << endl;
        file << "lifetimeEarnings=" << fixed << setprecision(2) << lifetimeEarnings << endl;
        
        // Save generator owned counts
        // Format: gen_0=5, gen_1=3, etc.
        for (size_t i = 0; i < generators.size(); i++) {
            file << "gen_" << i << "=" << generators[i].getOwned() << endl;
        }
        
        // Save upgrade levels
        // Format: upg_0=2, upg_1=1, etc.
        for (size_t i = 0; i < upgrades.size(); i++) {
            file << "upg_" << i << "=" << upgrades[i].getCurrentLevel() << endl;
        }
        
        // Save prestige data (Sprint 4)
        file << "prestigeLevel=" << prestigeLevel << endl;
        file << "prestigeMultiplier=" << fixed << setprecision(2) << prestigeMultiplier << endl;
        file << "totalLifetimeEarnings=" << fixed << setprecision(2) << totalLifetimeEarnings << endl;
        
        // Save achievements (Sprint 4)
        file << "achFirstWin=" << achFirstWin << endl;
        file << "achHighRoller=" << achHighRoller << endl;
        file << "achMillionaire=" << achMillionaire << endl;
        file << "achLuckyStreak=" << achLuckyStreak << endl;
        file << "achGenerator10=" << achGenerator10 << endl;
        file << "achAllUpgrades=" << achAllUpgrades << endl;
        file << "achPrestige1=" << achPrestige1 << endl;
        file << "achPrestige5=" << achPrestige5 << endl;
        file << "currentWinStreak=" << currentWinStreak << endl;
        
        // Save Phase B data
        file << "achievementMultiplier=" << fixed << setprecision(4) << achievementMultiplier << endl;
        file << "prestigeStars=" << prestigeStars << endl;
        file << "perkStartingMoney=" << perkStartingMoney << endl;
        file << "perkGeneratorDiscount=" << perkGeneratorDiscount << endl;
        file << "perkOfflineCap=" << perkOfflineCap << endl;
        file << "perkPassiveBoost=" << perkPassiveBoost << endl;
        
        // Save Phase C data
        file << "specializationId=" << specializationId << endl;
        file << "interestUnlocked=" << interestUnlocked << endl;
        file << "interestRate=" << fixed << setprecision(6) << interestRate << endl;
        for (int i = 0; i < 10; i++) {
            file << "milestone_" << i << "=" << milestoneUnlocked[i] << endl;
        }
        
        file.close();
        lastSaveTime = time(0);
        
        return true;
    }
    
    /*
     * loadGame() - Load game state from a file
     * 
     * Reads the save file and restores all game state.
     * Also calculates offline earnings based on time passed.
     * 
     * Returns: true if load was successful, false otherwise
     */
    bool loadGame() {
        // ifstream is "input file stream" - used for reading from files
        ifstream file(SAVE_FILE);
        
        if (!file.is_open()) {
            // No save file exists - this is normal for new players
            return false;
        }
        
        string line;
        time_t savedTimestamp = 0;
        
        // Read and verify header
        if (!getline(file, line) || line != "LUCKY_SPIN_SAVE_V1") {
            file.close();
            return false;  // Invalid or incompatible save file
        }
        
        // Parse each line
        while (getline(file, line)) {
            // Find the '=' separator
            size_t eqPos = line.find('=');
            if (eqPos == string::npos) continue;  // Skip invalid lines
            
            // Split into key and value
            string key = line.substr(0, eqPos);
            string value = line.substr(eqPos + 1);
            
            // Parse based on key
            // stod = string to double, stoi = string to int, stol = string to long
            if (key == "timestamp") {
                savedTimestamp = stol(value);
            } else if (key == "money") {
                money = stod(value);
            } else if (key == "betAmount") {
                betAmount = stod(value);
            } else if (key == "round") {
                round = stoi(value);
            } else if (key == "totalWins") {
                totalWins = stoi(value);
            } else if (key == "totalLosses") {
                totalLosses = stoi(value);
            } else if (key == "peakMoney") {
                peakMoney = stod(value);
            } else if (key == "lifetimeEarnings") {
                lifetimeEarnings = stod(value);
            } else if (key.substr(0, 4) == "gen_") {
                // Generator data: gen_0, gen_1, etc.
                int index = stoi(key.substr(4));
                if (index >= 0 && index < (int)generators.size()) {
                    generators[index].setOwned(stoi(value));
                }
            } else if (key.substr(0, 4) == "upg_") {
                // Upgrade data: upg_0, upg_1, etc.
                int index = stoi(key.substr(4));
                if (index >= 0 && index < (int)upgrades.size()) {
                    upgrades[index].setLevel(stoi(value));
                }
            }
            // Prestige data (Sprint 4)
            else if (key == "prestigeLevel") {
                prestigeLevel = stoi(value);
            } else if (key == "prestigeMultiplier") {
                prestigeMultiplier = stod(value);
            } else if (key == "totalLifetimeEarnings") {
                totalLifetimeEarnings = stod(value);
            }
            // Achievement data (Sprint 4)
            else if (key == "achFirstWin") {
                achFirstWin = (stoi(value) != 0);
            } else if (key == "achHighRoller") {
                achHighRoller = (stoi(value) != 0);
            } else if (key == "achMillionaire") {
                achMillionaire = (stoi(value) != 0);
            } else if (key == "achLuckyStreak") {
                achLuckyStreak = (stoi(value) != 0);
            } else if (key == "achGenerator10") {
                achGenerator10 = (stoi(value) != 0);
            } else if (key == "achAllUpgrades") {
                achAllUpgrades = (stoi(value) != 0);
            } else if (key == "achPrestige1") {
                achPrestige1 = (stoi(value) != 0);
            } else if (key == "achPrestige5") {
                achPrestige5 = (stoi(value) != 0);
            } else if (key == "currentWinStreak") {
                currentWinStreak = stoi(value);
            }
            // Phase B data
            else if (key == "achievementMultiplier") {
                achievementMultiplier = stod(value);
            } else if (key == "prestigeStars") {
                prestigeStars = stoi(value);
            } else if (key == "perkStartingMoney") {
                perkStartingMoney = stoi(value);
            } else if (key == "perkGeneratorDiscount") {
                perkGeneratorDiscount = stoi(value);
            } else if (key == "perkOfflineCap") {
                perkOfflineCap = stoi(value);
            } else if (key == "perkPassiveBoost") {
                perkPassiveBoost = stoi(value);
            }
            // Phase C data
            else if (key == "specializationId") {
                specializationId = stoi(value);
            } else if (key == "interestUnlocked") {
                interestUnlocked = (stoi(value) != 0);
            } else if (key == "interestRate") {
                interestRate = stod(value);
            } else if (key.substr(0, 10) == "milestone_") {
                int index = stoi(key.substr(10));
                if (index >= 0 && index < 10) {
                    milestoneUnlocked[index] = (stoi(value) != 0);
                }
            }
        }
        
        file.close();
        
        // Recalculate achievement multiplier in case it wasn't saved (backward compatibility)
        updateAchievementMultiplier();
        
        // Calculate and apply offline earnings
        if (savedTimestamp > 0) {
            applyOfflineEarnings(savedTimestamp);
        }
        
        // Update the last update time to now
        lastUpdateTime = time(0);
        lastSaveTime = time(0);
        
        return true;
    }
    
    /*
     * applyOfflineEarnings() - Calculate earnings while the game was closed
     * 
     * This is a key feature of incremental games - progress continues even
     * when you're not playing! We calculate how much time passed and apply
     * passive income for that duration.
     * 
     * Parameters:
     *   - savedTimestamp: When the game was last saved
     */
    void applyOfflineEarnings(time_t savedTimestamp) {
        time_t now = time(0);
        double secondsOffline = difftime(now, savedTimestamp);
        
        // Cap offline earnings (base 24 hours + 2 hours per perk level)
        // This prevents exploits and keeps the game balanced
        const double BASE_OFFLINE_HOURS = 24.0;
        double maxOfflineHours = BASE_OFFLINE_HOURS + (perkOfflineCap * 2.0);
        const double MAX_OFFLINE_SECONDS = maxOfflineHours * 3600.0;
        if (secondsOffline > MAX_OFFLINE_SECONDS) {
            secondsOffline = MAX_OFFLINE_SECONDS;
        }
        
        // Only apply if there's passive income and time has passed
        double passiveIncome = getTotalPassiveIncome();
        if (secondsOffline > 0 && passiveIncome > 0) {
            double offlineEarnings = passiveIncome * secondsOffline;
            money += offlineEarnings;
            lifetimeEarnings += offlineEarnings;
            updatePeakMoney();
            
            // Show welcome back message with earnings
            clearScreen();
            displayHeader();
            cout << "\n";
            printLine('-', 50, Color::GREEN);
            printCentered("WELCOME BACK!", 50, Color::GREEN + Color::BOLD);
            printLine('-', 50, Color::GREEN);
            
            cout << "\n  " << Color::WHITE << "You were away for: " << Color::CYAN;
            
            // Format time nicely
            int hours = (int)(secondsOffline / 3600);
            int minutes = (int)((secondsOffline - hours * 3600) / 60);
            int seconds = (int)(secondsOffline - hours * 3600 - minutes * 60);
            
            if (hours > 0) {
                cout << hours << "h " << minutes << "m " << seconds << "s";
            } else if (minutes > 0) {
                cout << minutes << "m " << seconds << "s";
            } else {
                cout << seconds << "s";
            }
            cout << Color::RESET << "\n";
            
            cout << "\n  " << Color::WHITE << "Offline earnings: " << Color::GREEN 
                 << "+" << formatMoney(offlineEarnings) << Color::RESET << "\n";
            cout << "  " << Color::DIM << "(at " << fixed << setprecision(2) 
                 << passiveIncome << "$/sec)" << Color::RESET << "\n";
            
            cout << "\n  " << Color::WHITE << "New balance: " << Color::GREEN 
                 << formatMoney(money) << Color::RESET << "\n";
            
            cout << "\n  Press any key to continue...";
            getch();
        }
    }
    
    /*
     * autoSave() - Automatically save the game if enough time has passed
     * 
     * Called periodically in the game loop. Saves every 30 seconds.
     */
    void autoSave() {
        const int AUTO_SAVE_INTERVAL = 30;  // seconds
        
        time_t now = time(0);
        if (difftime(now, lastSaveTime) >= AUTO_SAVE_INTERVAL) {
            saveGame();
            // Don't show message for auto-save to avoid interrupting gameplay
        }
    }
    
    /*
     * deleteSaveFile() - Delete the save file (for testing or reset)
     */
    void deleteSaveFile() {
        remove(SAVE_FILE.c_str());
    }
    
    // ========================================================================
    // PRESTIGE SYSTEM (Sprint 4 - Polish)
    // ========================================================================
    
    /*
     * getPrestigeRequirement() - Calculate money needed to prestige
     * 
     * Each prestige requires more money than the last.
     * Formula: 100,000 * (2 ^ prestigeLevel)
     */
    double getPrestigeRequirement() const {
        return 100000.0 * pow(2.0, prestigeLevel);
    }
    
    /*
     * canPrestige() - Check if player meets prestige requirements
     */
    bool canPrestige() const {
        return lifetimeEarnings >= getPrestigeRequirement();
    }
    
    /*
     * displayPrestigeMenu() - Show prestige information and options
     */
    void displayPrestigeMenu() {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('=', 50, Color::MAGENTA);
        printCentered("PRESTIGE", 50, Color::MAGENTA + Color::BOLD);
        printLine('=', 50, Color::MAGENTA);
        
        cout << "\n  " << Color::WHITE << "Current Prestige Level: " << Color::YELLOW 
             << prestigeLevel << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Current Multiplier: " << Color::GREEN 
             << fixed << setprecision(1) << prestigeMultiplier << "x" << Color::RESET << "\n";
        
        cout << "\n  " << Color::DIM << "Prestige resets your money, generators, and upgrades" << Color::RESET << "\n";
        cout << "  " << Color::DIM << "but gives you a permanent +10% income multiplier!" << Color::RESET << "\n";
        
        double requirement = getPrestigeRequirement();
        cout << "\n  " << Color::WHITE << "Lifetime Earnings: " << Color::CYAN 
             << formatMoney(lifetimeEarnings) << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Required to Prestige: " << Color::YELLOW 
             << formatMoney(requirement) << Color::RESET << "\n";
        
        double progress = (lifetimeEarnings / requirement) * 100.0;
        if (progress > 100.0) progress = 100.0;
        
        // Progress bar
        cout << "\n  Progress: [";
        int barWidth = 30;
        int filled = (int)(progress / 100.0 * barWidth);
        for (int i = 0; i < barWidth; i++) {
            if (i < filled) {
                cout << Color::GREEN << "█" << Color::RESET;
            } else {
                cout << Color::DIM << "░" << Color::RESET;
            }
        }
        cout << "] " << fixed << setprecision(1) << progress << "%\n";
        
        if (canPrestige()) {
            cout << "\n  " << Color::BG_GREEN << Color::WHITE << Color::BOLD 
                 << " PRESTIGE AVAILABLE! " << Color::RESET << "\n";
            cout << "\n  " << Color::WHITE << "After prestige you will have:" << Color::RESET << "\n";
            cout << "    - Money: " << Color::CYAN << "$100.00" << Color::RESET << "\n";
            cout << "    - Multiplier: " << Color::GREEN << fixed << setprecision(1) 
                 << (prestigeMultiplier + 0.1) << "x" << Color::RESET << "\n";
            cout << "\n  " << Color::CYAN << "[P]" << Color::RESET << " Prestige Now\n";
        } else {
            cout << "\n  " << Color::DIM << "Keep earning to unlock prestige!" << Color::RESET << "\n";
        }
        
        cout << "  " << Color::CYAN << "[P]" << Color::RESET << " Prestige Now";
        if (!canPrestige()) cout << Color::DIM << " (locked)";
        cout << Color::RESET << "\n";
        cout << "  " << Color::CYAN << "[S]" << Color::RESET << " Prestige Shop (" 
             << Color::YELLOW << prestigeStars << " ★" << Color::RESET << ")\n";
        cout << "  " << Color::CYAN << "[0]" << Color::RESET << " Back to Main Menu\n";
        cout << "\n  Choice: ";
        
        char choice = getch();
        
        if ((choice == 'p' || choice == 'P') && canPrestige()) {
            performPrestige();
        } else if (choice == 's' || choice == 'S') {
            displayPrestigeShop();
        }
    }
    
    /*
     * displayPrestigeShop() - Show prestige shop with permanent perks (Phase B)
     */
    void displayPrestigeShop() {
        bool inShop = true;
        int shopSelection = 0;
        const int PERK_COUNT = 4;
        
        // Perk definitions: name, description, max level, cost per level
        struct PerkInfo {
            string name;
            string desc;
            int maxLevel;
            int baseCost;
            int* currentLevel;
        };
        
        PerkInfo perks[PERK_COUNT] = {
            {"Starting Money", "Start with more money after prestige", 10, 3, &perkStartingMoney},
            {"Generator Discount", "All generators cost less", 10, 5, &perkGeneratorDiscount},
            {"Offline Cap", "Increase offline earnings cap", 5, 8, &perkOfflineCap},
            {"Passive Boost", "Permanent +5% passive income per level", 20, 4, &perkPassiveBoost}
        };
        
        while (inShop) {
            clearScreen();
            printLine('=', 50, Color::YELLOW);
            cout << Color::BOLD << Color::YELLOW;
            printCentered("PRESTIGE SHOP", 50);
            cout << Color::RESET;
            printLine('=', 50, Color::YELLOW);
            
            cout << "\n  " << Color::WHITE << "Prestige Stars: " << Color::YELLOW 
                 << prestigeStars << " ★" << Color::RESET << "\n";
            cout << "  " << Color::DIM << "Use W/S to navigate, Enter to buy, Q to exit" 
                 << Color::RESET << "\n\n";
            
            printLine('-', 50, Color::DIM);
            
            // Display perks
            for (int i = 0; i < PERK_COUNT; i++) {
                PerkInfo& perk = perks[i];
                int level = *(perk.currentLevel);
                bool maxed = (level >= perk.maxLevel);
                int cost = perk.baseCost + (level * 2);  // Cost increases with level
                bool canAfford = (prestigeStars >= cost) && !maxed;
                
                // Highlight selected item
                if (i == shopSelection) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                    cout << perk.name << Color::RESET;
                } else {
                    cout << "    " << Color::WHITE << perk.name << Color::RESET;
                }
                
                cout << " [" << Color::CYAN << level << "/" << perk.maxLevel << Color::RESET << "]";
                
                if (maxed) {
                    cout << " " << Color::GREEN << "MAX" << Color::RESET;
                } else {
                    string costColor = canAfford ? Color::GREEN : Color::RED;
                    cout << " - Cost: " << costColor << cost << " ★" << Color::RESET;
                }
                cout << "\n";
                
                cout << "      " << Color::DIM << perk.desc << Color::RESET << "\n";
                
                // Show current effect
                if (level > 0) {
                    cout << "      " << Color::GREEN << "Current: ";
                    if (i == 0) cout << "+$" << (level * 50);
                    else if (i == 1) cout << "-" << (level * 5) << "% cost";
                    else if (i == 2) cout << "+" << (level * 2) << "h cap";
                    else if (i == 3) cout << "+" << (level * 5) << "% income";
                    cout << Color::RESET << "\n";
                }
            }
            
            printLine('-', 50, Color::DIM);
            
            // Back option
            if (shopSelection == PERK_COUNT) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << "Back to Prestige Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::WHITE << "Back to Prestige Menu" << Color::RESET << "\n";
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
                shopSelection = (shopSelection - 1 + PERK_COUNT + 1) % (PERK_COUNT + 1);
            } else if (choice == 's' || choice == 'S') {
                shopSelection = (shopSelection + 1) % (PERK_COUNT + 1);
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (shopSelection == PERK_COUNT) {
                    inShop = false;
                } else {
                    // Try to purchase perk
                    PerkInfo& perk = perks[shopSelection];
                    int level = *(perk.currentLevel);
                    if (level < perk.maxLevel) {
                        int cost = perk.baseCost + (level * 2);
                        if (prestigeStars >= cost) {
                            prestigeStars -= cost;
                            (*(perk.currentLevel))++;
                            cout << "\n  " << Color::GREEN << UI::CHECK << " Purchased " 
                                 << perk.name << " (Level " << (*(perk.currentLevel)) << ")!" << Color::RESET;
                            usleep(800000);
                        } else {
                            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough stars! Need " 
                                 << cost << " ★" << Color::RESET;
                            usleep(800000);
                        }
                    } else {
                        cout << "\n  " << Color::YELLOW << "Already at max level!" << Color::RESET;
                        usleep(800000);
                    }
                }
            } else if (choice == 'q' || choice == 'Q' || choice == 'a' || choice == 'A') {
                inShop = false;
            }
        }
    }
    
    /*
     * displaySpecializationMenu() - Choose a specialization build (Phase C)
     */
    void displaySpecializationMenu() {
        clearScreen();
        printLine('=', 50, Color::CYAN);
        cout << Color::BOLD << Color::CYAN;
        printCentered("SPECIALIZATION", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::CYAN);
        
        cout << "\n  " << Color::WHITE << "Choose your casino specialty for bonus payouts!" << Color::RESET << "\n";
        cout << "  " << Color::DIM << "You can change this anytime, but bonuses only apply to your specialty." << Color::RESET << "\n\n";
        
        const char* specNames[] = {"None", "Wheel Tycoon", "Slot Mogul", "Card Shark", "Roulette Baron"};
        const char* specDesc[] = {
            "No specialization bonus",
            "+50% payout on Wheel spins",
            "+50% payout on Slots",
            "+50% payout on Blackjack",
            "+50% payout on Roulette"
        };
        const char* specIcons[] = {"", "🎡", "🎰", "🃏", "🎲"};
        
        for (int i = 0; i <= 4; i++) {
            if (i == specializationId) {
                cout << "  " << Color::BG_GREEN << Color::WHITE << Color::BOLD << " ✓ ";
            } else {
                cout << "    ";
            }
            
            cout << Color::CYAN << "[" << i << "]" << Color::RESET << " ";
            if (i > 0) cout << specIcons[i] << " ";
            cout << Color::WHITE << specNames[i] << Color::RESET;
            
            if (i == specializationId) {
                cout << Color::BG_GREEN << " " << Color::RESET;
            }
            cout << "\n";
            
            cout << "      " << Color::DIM << specDesc[i] << Color::RESET << "\n\n";
        }
        
        cout << "  " << Color::CYAN << "[Q]" << Color::RESET << " Back to Main Menu\n";
        cout << "\n  Choice: ";
        
        char choice = getch();
        
        if (choice >= '0' && choice <= '4') {
            int newSpec = choice - '0';
            if (newSpec != specializationId) {
                specializationId = newSpec;
                cout << "\n  " << Color::GREEN << UI::CHECK << " Specialization changed to: " 
                     << specNames[newSpec] << Color::RESET;
                usleep(1000000);
            }
        }
    }
    
    /*
     * calculatePrestigeStars() - Calculate stars earned from prestige (Phase B)
     * 
     * Formula: base 5 stars + 1 per prestige level + bonus from lifetime earnings
     */
    int calculatePrestigeStars() const {
        int baseStars = 5;
        int levelBonus = prestigeLevel;
        int earningsBonus = (int)(totalLifetimeEarnings / 1000000.0);  // 1 star per million
        return baseStars + levelBonus + earningsBonus;
    }
    
    /*
     * getStartingMoneyFromPerk() - Calculate starting money based on perk level (Phase B)
     */
    double getStartingMoneyFromPerk() const {
        return 100.0 + (perkStartingMoney * 50.0);  // +$50 per level
    }
    
    /*
     * performPrestige() - Execute the prestige reset
     */
    void performPrestige() {
        // Confirm prestige
        cout << "\n\n  " << Color::YELLOW << "Are you sure? This will reset your progress!" 
             << Color::RESET << "\n";
        cout << "  " << Color::CYAN << "[Y]" << Color::RESET << " Yes  "
             << Color::CYAN << "[N]" << Color::RESET << " No\n";
        cout << "  Choice: ";
        
        char confirm = getch();
        if (confirm != 'y' && confirm != 'Y') {
            return;
        }
        
        // Calculate prestige stars earned (Phase B)
        int starsEarned = calculatePrestigeStars();
        prestigeStars += starsEarned;
        
        // Perform prestige
        prestigeLevel++;
        prestigeMultiplier += 0.1;
        totalLifetimeEarnings += lifetimeEarnings;
        
        // Reset progress (apply starting money perk)
        money = getStartingMoneyFromPerk();
        betAmount = 10.0;
        round = 1;
        totalWins = 0;
        totalLosses = 0;
        peakMoney = money;
        lifetimeEarnings = 0.0;
        currentWinStreak = 0;
        
        // Reset generators
        for (auto& gen : generators) {
            gen.setOwned(0);
        }
        
        // Reset upgrades
        for (auto& upg : upgrades) {
            upg.setLevel(0);
        }
        
        // Check prestige achievements (Phase B)
        checkPrestigeAchievements();
        
        // Show success message
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('*', 50, Color::YELLOW);
        printCentered("PRESTIGE COMPLETE!", 50, Color::YELLOW + Color::BOLD);
        printLine('*', 50, Color::YELLOW);
        
        cout << "\n  " << Color::WHITE << "You are now Prestige Level " << Color::YELLOW 
             << prestigeLevel << Color::RESET << "!\n";
        cout << "  " << Color::WHITE << "New income multiplier: " << Color::GREEN 
             << fixed << setprecision(1) << prestigeMultiplier << "x" << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Prestige stars earned: " << Color::YELLOW 
             << "+" << starsEarned << " ★" << Color::RESET << "\n";
        cout << "  " << Color::WHITE << "Total stars: " << Color::YELLOW 
             << prestigeStars << " ★" << Color::RESET << "\n";
        
        cout << "\n  Press any key to continue...";
        getch();
        
        saveGame();
    }
    
    // ========================================================================
    // ACHIEVEMENT SYSTEM (Sprint 4 - Polish)
    // ========================================================================
    
    /*
     * showAchievement() - Display achievement unlock notification
     */
    void showAchievement(const string& name, const string& desc) {
        cout << "\n";
        cout << "  " << Color::BG_YELLOW << Color::BLACK << Color::BOLD 
             << " ★ ACHIEVEMENT UNLOCKED! ★ " << Color::RESET << "\n";
        cout << "  " << Color::YELLOW << name << Color::RESET << "\n";
        cout << "  " << Color::DIM << desc << Color::RESET << "\n";
        usleep(1500000);
    }
    
    /*
     * updateAchievementMultiplier() - Recalculate achievement bonus (Phase B)
     * 
     * Each achievement gives +1% to all passive income.
     */
    void updateAchievementMultiplier() {
        int count = getAchievementCount();
        achievementMultiplier = 1.0 + (count * 0.01);
    }
    
    /*
     * checkAchievements() - Check and unlock any earned achievements
     * 
     * Called after wins and purchases to check for new achievements.
     */
    void checkAchievements() {
        bool anyNewAchievement = false;
        
        // First Win
        if (!achFirstWin && totalWins >= 1) {
            achFirstWin = true;
            showAchievement("First Win", "Win your first game! (+1% income)");
            anyNewAchievement = true;
        }
        
        // Millionaire
        if (!achMillionaire && money >= 1000000) {
            achMillionaire = true;
            showAchievement("Millionaire", "Reach $1,000,000! (+1% income)");
            anyNewAchievement = true;
        }
        
        // Lucky Streak
        if (!achLuckyStreak && currentWinStreak >= 10) {
            achLuckyStreak = true;
            showAchievement("Lucky Streak", "Win 10 games in a row! (+1% income)");
            anyNewAchievement = true;
        }
        
        // Generator 10
        if (!achGenerator10) {
            for (const auto& gen : generators) {
                if (gen.getOwned() >= 10) {
                    achGenerator10 = true;
                    showAchievement("Generator Tycoon", "Own 10 of any generator! (+1% income)");
                    anyNewAchievement = true;
                    break;
                }
            }
        }
        
        // All Upgrades Maxed
        if (!achAllUpgrades) {
            bool allMaxed = true;
            for (const auto& upg : upgrades) {
                if (!upg.isMaxed()) {
                    allMaxed = false;
                    break;
                }
            }
            if (allMaxed && upgrades.size() > 0) {
                achAllUpgrades = true;
                showAchievement("Fully Upgraded", "Max out all upgrades! (+1% income)");
                anyNewAchievement = true;
            }
        }
        
        // Update multiplier if any new achievements were unlocked
        if (anyNewAchievement) {
            updateAchievementMultiplier();
        }
    }
    
    /*
     * checkHighRollerAchievement() - Check for high roller achievement
     * 
     * Called after a win with the win amount.
     */
    void checkHighRollerAchievement(double winAmount) {
        if (!achHighRoller && winAmount >= 1000) {
            achHighRoller = true;
            showAchievement("High Roller", "Win $1,000 in a single bet! (+1% income)");
            updateAchievementMultiplier();
        }
    }
    
    /*
     * checkPrestigeAchievements() - Check prestige-related achievements (Phase B)
     * 
     * Called after performing prestige.
     */
    void checkPrestigeAchievements() {
        bool anyNewAchievement = false;
        
        if (!achPrestige1 && prestigeLevel >= 1) {
            achPrestige1 = true;
            showAchievement("First Prestige", "Prestige for the first time! (+1% income)");
            anyNewAchievement = true;
        }
        if (!achPrestige5 && prestigeLevel >= 5) {
            achPrestige5 = true;
            showAchievement("Prestige Master", "Reach prestige level 5! (+1% income)");
            anyNewAchievement = true;
        }
        
        if (anyNewAchievement) {
            updateAchievementMultiplier();
        }
    }
    
    /*
     * displayAchievements() - Show all achievements and their status
     */
    void displayAchievements() {
        clearScreen();
        displayHeader();
        cout << "\n";
        printLine('=', 50, Color::YELLOW);
        printCentered("ACHIEVEMENTS", 50, Color::YELLOW + Color::BOLD);
        printLine('=', 50, Color::YELLOW);
        
        int unlocked = 0;
        int total = 8;
        
        auto showAch = [&](bool earned, const string& name, const string& desc) {
            if (earned) {
                unlocked++;
                cout << "  " << Color::GREEN << "★ " << Color::RESET;
            } else {
                cout << "  " << Color::DIM << "☆ " << Color::RESET;
            }
            cout << (earned ? Color::WHITE : Color::DIM) << name << Color::RESET;
            cout << "\n    " << Color::DIM << desc << Color::RESET << "\n\n";
        };
        
        cout << "\n";
        showAch(achFirstWin, "First Win", "Win your first game");
        showAch(achHighRoller, "High Roller", "Win $1,000 in a single bet");
        showAch(achMillionaire, "Millionaire", "Reach $1,000,000");
        showAch(achLuckyStreak, "Lucky Streak", "Win 10 games in a row");
        showAch(achGenerator10, "Generator Tycoon", "Own 10 of any generator");
        showAch(achAllUpgrades, "Fully Upgraded", "Max out all upgrades");
        showAch(achPrestige1, "First Prestige", "Prestige for the first time");
        showAch(achPrestige5, "Prestige Master", "Reach prestige level 5");
        
        printLine('-', 50, Color::DIM);
        cout << "\n  " << Color::WHITE << "Progress: " << Color::YELLOW 
             << unlocked << "/" << total << Color::RESET << " achievements unlocked\n";
        
        cout << "\n  Press any key to continue...";
        getch();
    }
    
    /*
     * recordWin() - Record a win and update streak
     */
    void recordWin(double winAmount) {
        totalWins++;
        currentWinStreak++;
        updatePeakMoney();
        checkHighRollerAchievement(winAmount);
        checkAchievements();
    }
    
    /*
     * recordLoss() - Record a loss and reset streak
     */
    void recordLoss() {
        totalLosses++;
        currentWinStreak = 0;
    }
    
    // ========================================================================
    // GENERATOR SHOP (Sprint 1 - Core Incremental Feature)
    // ========================================================================
    
    /*
     * getGeneratorCostMultiplier() - Get cost reduction from prestige perks (Phase B)
     */
    double getGeneratorCostMultiplier() const {
        return 1.0 - (perkGeneratorDiscount * 0.05);  // -5% per level
    }
    
    double getGeneratorBulkCost(const Generator& gen, int amount) const {
        if (amount <= 0) return 0.0;
        double currentCost = gen.getCurrentCost();
        double multiplier = gen.getCostMultiplier();
        if (!std::isfinite(currentCost) || currentCost <= 0.0) return 0.0;
        if (!std::isfinite(multiplier) || multiplier <= 1.0) return currentCost * amount;
        double baseCost = currentCost * (pow(multiplier, amount) - 1.0) / (multiplier - 1.0);
        // Apply generator discount perk (Phase B)
        return baseCost * getGeneratorCostMultiplier();
    }

    int getGeneratorMaxAffordable(const Generator& gen, double availableMoney) const {
        if (!std::isfinite(availableMoney) || availableMoney <= 0.0) return 0;

        double currentCost = gen.getCurrentCost();
        double multiplier = gen.getCostMultiplier();
        if (!std::isfinite(currentCost) || currentCost <= 0.0) return 0;

        int owned = gen.getOwned();
        if (owned >= INT_MAX) return 0;
        int maxPossible = INT_MAX - owned;

        if (!std::isfinite(multiplier) || multiplier <= 1.0) {
            int n = (int)floor(availableMoney / currentCost);
            if (n < 0) n = 0;
            if (n > maxPossible) n = maxPossible;
            return n;
        }

        double a = availableMoney * (multiplier - 1.0) / currentCost + 1.0;
        if (!std::isfinite(a) || a <= 1.0) return 0;

        double nExact = log(a) / log(multiplier);
        if (!std::isfinite(nExact)) return 0;

        int n = (int)floor(nExact);
        if (n < 0) n = 0;
        if (n > maxPossible) n = maxPossible;

        while (n > 0 && getGeneratorBulkCost(gen, n) > availableMoney) {
            n--;
        }
        while (n < maxPossible && getGeneratorBulkCost(gen, n + 1) <= availableMoney) {
            n++;
        }

        return n;
    }
    
    /*
     * displayGeneratorShop() - Show the generator purchase menu
     * 
     * This is where players spend money to buy passive income generators.
     * Each generator produces money automatically over time.
     * 
     * The shop displays:
     * - Generator name and how many owned
     * - Cost to buy the next one
     * - Income per second each one provides
     */
    void displayGeneratorShop() {
        bool inShop = true;
        int shopSelection = 0;  // 0 = back, 1-8 = generators
        int maxSelection = generators.size();  // 0 to maxSelection

        int buyMode = 0;
        static const int BUY_MODE_COUNT = 5;
        const int buyAmounts[BUY_MODE_COUNT] = {1, 10, 25, 100, 0};
        const string buyModeNames[BUY_MODE_COUNT] = {"x1", "x10", "x25", "x100", "MAX"};
         
        while (inShop) {
            // Update passive income while in shop
            updatePassiveIncome();
            
            clearScreen();
            printLine('=', 50, Color::MAGENTA);
            cout << Color::BOLD << Color::MAGENTA;
            printCentered("GENERATOR SHOP", 50);
            cout << Color::RESET;
            printLine('=', 50, Color::MAGENTA);
            
            // Show current balance and income
            cout << "\n  " << Color::WHITE << "Your Balance: " << Color::GREEN 
                 << formatMoney(money) << Color::RESET << "\n";
            cout << "  " << Color::WHITE << "Current Income: " << Color::GREEN 
                 << "+$" << fixed << setprecision(2) << getTotalPassiveIncome() 
                 << "/sec" << Color::RESET << "\n";
            cout << "  " << Color::WHITE << "Buy Mode: " << Color::CYAN 
                 << buyModeNames[buyMode] << Color::RESET << "  " << Color::DIM
                 << "(press E to change)" << Color::RESET << "\n";
            cout << "  " << Color::DIM << "Use W/S to navigate, Enter to buy, E to change quantity, Q to exit" 
                 << Color::RESET << "\n\n";
             
            printLine('-', 50, Color::DIM);
             
            // List all generators with cursor
            for (size_t i = 0; i < generators.size(); i++) {
                Generator& gen = generators[i];

                int buyAmount = buyAmounts[buyMode];
                int amountToBuy = buyAmount;
                double cost = 0.0;

                if (buyAmount == 0) {
                    amountToBuy = getGeneratorMaxAffordable(gen, money);
                    if (amountToBuy > 0) {
                        cost = getGeneratorBulkCost(gen, amountToBuy);
                    } else {
                        cost = gen.getCurrentCost();
                    }
                } else {
                    cost = getGeneratorBulkCost(gen, buyAmount);
                }

                bool canAfford = (buyAmount == 0) ? (amountToBuy > 0 && money >= cost) : (money >= cost);
                string costColor = canAfford ? Color::GREEN : Color::RED;

                double perUnitIncome = gen.getBaseIncome() * getIncomeMultiplier() * prestigeMultiplier;
                double gainPerSec = perUnitIncome * ((buyAmount == 0) ? amountToBuy : buyAmount);

                string buyLabel;
                if (buyAmount == 0) {
                    buyLabel = "MAX x" + to_string(amountToBuy);
                } else {
                    buyLabel = "x" + to_string(buyAmount);
                }
                 
                // Highlight selected item
                if ((int)i + 1 == shopSelection) {
                    cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                    cout << gen.getIcon() << " " << gen.getName() << Color::RESET;
                } else {
                    cout << "    " << gen.getIcon() << " " << Color::WHITE << gen.getName() << Color::RESET;
                }
                
                cout << " x" << gen.getOwned();
                cout << " " << Color::GREEN << "(+$" << fixed << setprecision(1) 
                     << gen.getBaseIncome() << "/s)" << Color::RESET << "\n";
                 
                cout << "      " << Color::DIM << gen.getDescription() << Color::RESET << "\n";
                cout << "      Buy: " << Color::CYAN << buyLabel << Color::RESET;
                cout << "  Cost: " << costColor << formatMoney(cost) << Color::RESET;
                cout << "  Gain: " << Color::GREEN << "+" << formatMoney(gainPerSec) << "/sec" << Color::RESET << "\n";
            }
            
            printLine('-', 50, Color::DIM);
            
            // Back option
            if (shopSelection == 0) {
                cout << "  " << Color::BG_BLUE << Color::WHITE << Color::BOLD << " " << UI::ARROW_RIGHT << " ";
                cout << "Back to Main Menu" << Color::RESET << "\n";
            } else {
                cout << "    " << Color::WHITE << "Back to Main Menu" << Color::RESET << "\n";
            }
            
            // Get user input with timeout for real-time updates
            char choice = getchWithTimeout(1000);
            
            if (choice == 0) continue;  // Timeout, refresh
            
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
                shopSelection = (shopSelection - 1 + maxSelection + 1) % (maxSelection + 1);
            } else if (choice == 's' || choice == 'S') {
                shopSelection = (shopSelection + 1) % (maxSelection + 1);
            } else if (choice == 'e' || choice == 'E') {
                buyMode = (buyMode + 1) % BUY_MODE_COUNT;
            } else if (choice == '\n' || choice == '\r' || choice == ' ' || choice == 'd' || choice == 'D') {
                if (shopSelection == 0) {
                    inShop = false;
                } else {
                    purchaseGenerator(shopSelection - 1, buyAmounts[buyMode]);
                }
            } else if (choice == 'q' || choice == 'Q' || choice == 'a' || choice == 'A') {
                inShop = false;
            } else if (choice >= '1' && choice <= '0' + (int)generators.size()) {
                purchaseGenerator(choice - '1', buyAmounts[buyMode]);
            } else if (choice == '0') {
                inShop = false;
            }
        }
    }
    
    /*
     * purchaseGenerator() - Attempt to buy a generator
     * 
     * Parameters:
     *   - index: Index of the generator in the generators vector
     * 
     * This method:
     * 1. Checks if the player can afford the generator
     * 2. Deducts the cost from player's money
     * 3. Adds one to the generator's owned count
     */
    void purchaseGenerator(int index, int amount = 1) {
        // Bounds checking - make sure index is valid
        if (index < 0 || index >= (int)generators.size()) {
            return;
        }
        
        Generator& gen = generators[index];

        int purchaseAmount = amount;
        if (purchaseAmount < 0) purchaseAmount = 1;

        if (purchaseAmount == 0) {
            purchaseAmount = getGeneratorMaxAffordable(gen, money);
            if (purchaseAmount <= 0) {
                double singleCost = gen.getCurrentCost();
                cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need " 
                     << formatMoney(singleCost) << Color::RESET;
                usleep(800000);
                return;
            }
        }

        if (purchaseAmount > INT_MAX - gen.getOwned()) {
            purchaseAmount = INT_MAX - gen.getOwned();
        }
        if (purchaseAmount <= 0) {
            return;
        }

        double cost = getGeneratorBulkCost(gen, purchaseAmount);
         
        if (money >= cost) {
            money -= cost;
            gen.setOwned(gen.getOwned() + purchaseAmount);
             
            cout << "\n  " << Color::GREEN << UI::CHECK << " Purchased ";
            if (purchaseAmount == 1) {
                cout << gen.getName();
            } else {
                cout << purchaseAmount << "x " << gen.getName();
            }
            cout << "!" << Color::RESET;
            cout << "\n  " << Color::DIM << "You now own " << gen.getOwned() 
                 << " and earn +$" << fixed << setprecision(2) 
                 << gen.getIncomePerSecond() << "/sec from them." << Color::RESET;
             
            usleep(800000);
        } else {
            cout << "\n  " << Color::RED << UI::CROSS_X << " Not enough money! Need " 
                 << formatMoney(cost) << Color::RESET;
            usleep(800000);
        }
    }
    
    /*
     * confirmRestart() - Ask for confirmation before resetting all progress
     */
    void confirmRestart() {
        clearScreen();
        printLine('=', 50, Color::YELLOW);
        cout << Color::BOLD << Color::YELLOW;
        printCentered("RESTART GAME", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::YELLOW);
        
        cout << "\n";
        printCentered("Are you sure you want to restart?", 50, Color::WHITE);
        cout << "\n";
        cout << "  " << Color::RED << "WARNING: " << Color::RESET 
             << "This will delete ALL progress including:\n";
        cout << "    - Money and bet amount\n";
        cout << "    - All generators and upgrades\n";
        cout << "    - Prestige level and multiplier\n";
        cout << "    - Statistics and achievements\n";
        cout << "    - Save file\n";
        cout << "\n";
        printCentered("This action CANNOT be undone!", 50, Color::RED + Color::BOLD);
        cout << "\n\n";
        cout << "  " << Color::GREEN << "[Y]" << Color::RESET << " Yes, restart from scratch\n";
        cout << "  " << Color::RED << "[N]" << Color::RESET << " No, keep playing\n";
        cout << "\n  Your choice: ";
        
        char choice = getch();
        
        if (choice == 'y' || choice == 'Y') {
            resetGame();
            cout << "\n\n  " << Color::GREEN << UI::CHECK << " Game has been reset!" << Color::RESET;
            cout << "\n  " << Color::DIM << "Starting fresh with $100..." << Color::RESET;
            usleep(2000000);
        } else {
            cout << "\n\n  " << Color::CYAN << "Restart cancelled." << Color::RESET;
            usleep(1000000);
        }
    }
    
    /*
     * resetGame() - Reset all game state to initial values
     */
    void resetGame() {
        // Reset money and betting
        money = 100.0;
        betAmount = 10.0;
        peakMoney = 100.0;
        
        // Reset statistics
        round = 0;
        totalWins = 0;
        totalLosses = 0;
        currentWinStreak = 0;
        
        // Reset prestige
        prestigeLevel = 0;
        prestigeMultiplier = 1.0;
        lifetimeEarnings = 0.0;
        totalLifetimeEarnings = 0.0;
        
        // Reset generators
        for (auto& gen : generators) {
            gen.reset();
        }
        
        // Reset upgrades
        for (auto& upg : upgrades) {
            upg.reset();
        }
        
        // Reset achievements
        achFirstWin = false;
        achHighRoller = false;
        achLuckyStreak = false;
        achMillionaire = false;
        achPrestige1 = false;
        achPrestige5 = false;
        achGenerator10 = false;
        achAllUpgrades = false;
        
        // Reset menu and animation state
        menuSelection = 0;
        animationFrame = 0;
        
        // Reset time tracking
        lastUpdateTime = time(0);
        lastSaveTime = time(0);
        
        // Delete save file
        remove(SAVE_FILE.c_str());
    }
    
    /*
     * displayGameOver() - Show game over screen when player goes bankrupt
     */
    void displayGameOver() {
        clearScreen();
        printLine('=', 50, Color::RED);
        cout << Color::BOLD << Color::RED;
        printCentered("GAME OVER", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::RED);
        
        cout << "\n";
        printCentered("You went bankrupt!", 50, Color::RED);
        cout << "\n";
        cout << "  " << Color::WHITE << "Final Statistics:" << Color::RESET << "\n";
        cout << "    Rounds played: " << Color::CYAN << round << Color::RESET << "\n";
        cout << "    Total wins: " << Color::GREEN << totalWins << Color::RESET << "\n";
        cout << "    Total losses: " << Color::RED << totalLosses << Color::RESET << "\n";
        cout << "    Peak balance: " << Color::YELLOW << "$" << fixed 
             << setprecision(2) << peakMoney << Color::RESET << "\n";
        cout << "\n";
        printCentered("Better luck next time!", 50, Color::DIM);
        cout << "\n";
    }
    
    /*
     * displayExitScreen() - Show exit screen when player quits
     */
    void displayExitScreen() {
        clearScreen();
        printLine('=', 50, Color::GREEN);
        cout << Color::BOLD << Color::GREEN;
        printCentered("THANKS FOR PLAYING!", 50);
        cout << Color::RESET;
        printLine('=', 50, Color::GREEN);
        
        cout << "\n";
        cout << "  " << Color::WHITE << "Final Statistics:" << Color::RESET << "\n";
        cout << "    Final balance: " << Color::GREEN << "$" << fixed 
             << setprecision(2) << money << Color::RESET << "\n";
        cout << "    Rounds played: " << Color::CYAN << round << Color::RESET << "\n";
        cout << "    Total wins: " << Color::GREEN << totalWins << Color::RESET << "\n";
        cout << "    Total losses: " << Color::RED << totalLosses << Color::RESET << "\n";
        cout << "    Peak balance: " << Color::YELLOW << "$" << fixed 
             << setprecision(2) << peakMoney << Color::RESET << "\n";
        
        // Calculate and display profit/loss
        double profit = money - 100.0;  // Started with $100
        cout << "    Net profit: ";
        if (profit >= 0) {
            cout << Color::GREEN << "+$" << fixed << setprecision(2) << profit;
        } else {
            cout << Color::RED << "-$" << fixed << setprecision(2) << (-profit);
        }
        cout << Color::RESET << "\n\n";
    }
    
    // ========================================================================
    // MAIN GAME LOOP
    // ========================================================================
    
    /*
     * play() - Main game loop
     * 
     * This is the entry point for the game. It:
     * 1. Updates passive income based on elapsed time
     * 2. Displays the current game state
     * 3. Waits for player input (with timeout for continuous updates)
     * 4. Processes the input (play game, buy generators, change bet, quit)
     * 5. Checks for game over condition
     * 6. Repeats until player quits or goes bankrupt
     * 
     * The while loop continues as long as:
     * - quit is false (player hasn't chosen to quit)
     * - money > 0 OR passive income > 0 (can recover with generators)
     */
    void play() {
        char choice;      // Store the player's menu selection
        bool quit = false; // Flag to track if player wants to quit
        
        // Main game loop - continues while not quitting
        // With passive income, player can recover even from $0
        while (!quit) {
            // Update passive income before displaying
            // This is crucial for the incremental game feel
            updatePassiveIncome();
            
            displayStatus();
            
            // Use non-blocking input with timeout for real-time updates (Sprint 6)
            // This allows the balance to update visually every second
            choice = getchWithTimeout(1000);  // 1 second timeout
            
            // If timeout (no input), just refresh the display
            if (choice == 0) {
                continue;
            }
            
            // Handle arrow keys (escape sequences) and WASD navigation (Sprint 7)
            // Arrow keys send: ESC [ A (up), ESC [ B (down)
            if (choice == 27) {  // ESC character
                char seq1 = getchWithTimeout(50);
                if (seq1 == '[') {
                    char seq2 = getchWithTimeout(50);
                    if (seq2 == 'A') choice = 'w';  // Up arrow -> w
                    else if (seq2 == 'B') choice = 's';  // Down arrow -> s
                }
                if (choice == 27) continue;  // Just ESC pressed, ignore
            }
            
            // Navigation controls (Sprint 7)
            if (choice == 'w' || choice == 'W') {
                menuSelection = (menuSelection - 1 + MENU_ITEMS) % MENU_ITEMS;
                continue;
            }
            if (choice == 's' || choice == 'S') {
                // S always navigates down (use Enter to select Save)
                menuSelection = (menuSelection + 1) % MENU_ITEMS;
                continue;
            }
            if (choice == 'a' || choice == 'A') {
                // A can be used to go back/cancel in future
                continue;
            }
            if (choice == 'd' || choice == 'D' || choice == '\n' || choice == '\r' || choice == ' ') {
                // D, Enter, or Space selects current menu item
                choice = '0' + menuSelection;  // Convert to number for switch
                if (menuSelection >= 10) {
                    // Handle Save, Restart, and Quit specially
                    if (menuSelection == 10) choice = 's';
                    if (menuSelection == 11) choice = 'r';
                    if (menuSelection == 12) choice = 'q';
                }
            }
            
            // Also support direct number/letter input for quick access
            // Switch statement handles menu actions
            // Menu items: 0=Wheel, 1=Slots, 2=Blackjack, 3=Roulette, 4=GenShop, 5=UpgShop, 6=Bet, 7=Prestige, 8=Specialization, 9=Achievements
            switch(choice) {
                case '0':  // Spin Wheel (menuSelection 0)
                    spinWheel();
                    break;
                case '1':  // Play Slots (menuSelection 1)
                    playSlots();
                    break;
                case '2':  // Blackjack (menuSelection 2)
                    playBlackjack();
                    break;
                case '3':  // Roulette (menuSelection 3)
                    playRoulette();
                    break;
                case '4':  // Generator Shop (menuSelection 4)
                    displayGeneratorShop();
                    break;
                case '5':  // Upgrade Shop (menuSelection 5)
                    displayUpgradeShop();
                    break;
                case '6':  // Change Bet (menuSelection 6)
                    changeBet();
                    break;
                case '7':  // Prestige (menuSelection 7)
                    displayPrestigeMenu();
                    break;
                case '8':  // Specialization (menuSelection 8)
                    displaySpecializationMenu();
                    break;
                case '9':  // Achievements (menuSelection 9)
                    displayAchievements();
                    break;
                case 's':
                case 'S':
                    // Manual save (Sprint 3)
                    if (saveGame()) {
                        cout << "\n  " << Color::GREEN << UI::CHECK << " Game saved!" << Color::RESET;
                        usleep(1000000);
                    }
                    break;
                case 'r':
                case 'R':
                    // Restart game from scratch
                    confirmRestart();
                    break;
                case 'q':
                case 'Q':
                    quit = true;
                    break;
                default:
                    // Handle invalid input - don't show error, just refresh
                    break;
            }
            
            // Auto-save periodically (Sprint 3)
            autoSave();
            
            // Check for bankruptcy (only if no passive income to save them)
            if (money <= 0 && getTotalPassiveIncome() <= 0) {
                displayGameOver();
                cout << "\n  " << Color::YELLOW << "Press any key to start a new game..." << Color::RESET;
                getch();
                resetGame();  // Auto-reset so player can start fresh
                // Don't quit - continue playing with fresh start
            }
        }
        
        // Save game before exiting (Sprint 3)
        saveGame();
        
        // Show exit screen if player quit with money remaining
        if (money > 0) {
            displayExitScreen();
        }
    }
};

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================
/*
 * Static member variables must be defined outside the class.
 * This is because static members are shared across all instances and need
 * a single definition in memory.
 */
const string GamblingGame::SAVE_FILE = "lucky_spin_save.txt";

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
    // Create an instance (object) of the GamblingGame class
    // This calls the constructor, initializing all member variables
    GamblingGame game;
    
    // Try to load existing save file (Sprint 3)
    // If successful, offline earnings will be calculated and shown
    game.loadGame();
    
    // Start the game by calling the play() method
    game.play();
    
    // Return 0 to indicate successful program completion
    return 0;
}
