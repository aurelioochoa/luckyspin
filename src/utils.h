/*
 * utils.h - Utility functions for Lucky Spin Casino
 * 
 * Contains terminal I/O functions, formatting helpers, and other utilities.
 */

#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <cstring>
#include <cstdlib>
#include <csignal>
#include <cerrno>
#include "colors.h"
#include "core/text_width.h"

/*
 * ============================================================================
 * TERMINAL STATE OWNERSHIP
 * ============================================================================
 *
 * The game puts the terminal into a non-canonical, no-echo mode to read single
 * keypresses. If the process dies while in that mode (Ctrl+C, SIGTERM, an
 * uncaught exception) the user's shell is left with no echo and no line
 * editing, and they have to type `reset` blind.
 *
 * terminal::init() snapshots the original termios ONCE at startup and arranges
 * for it to be restored on normal exit and on fatal signals.
 */
namespace terminal {

inline struct termios& savedState() {
    static struct termios state;
    return state;
}

inline bool& haveSavedState() {
    static bool have = false;
    return have;
}

// Restore the terminal to how we found it. Safe to call more than once.
// tcsetattr() and write() are async-signal-safe, so this is usable in a handler.
inline void restore() {
    if (haveSavedState()) {
        tcsetattr(STDIN_FILENO, TCSANOW, &savedState());
    }
    static const char showCursor[] = "\033[?25h";
    ssize_t ignored = write(STDOUT_FILENO, showCursor, sizeof(showCursor) - 1);
    (void)ignored;
}

// Restore, then let the signal do what it would have done by default.
inline void onFatalSignal(int sig) {
    restore();
    signal(sig, SIG_DFL);
    raise(sig);
}

inline void init() {
    if (tcgetattr(STDIN_FILENO, &savedState()) == 0) {
        haveSavedState() = true;
    }
    std::atexit(restore);
    signal(SIGINT,  onFatalSignal);
    signal(SIGTERM, onFatalSignal);
    signal(SIGQUIT, onFatalSignal);
    signal(SIGHUP,  onFatalSignal);
}

} // namespace terminal

/*
 * getch() - Get a single character from the terminal without waiting for Enter
 *
 * Temporarily disables canonical mode and echo, reads one byte, then restores
 * the PREVIOUS settings from a saved copy.
 *
 * Returns: The character pressed, or 0 on error/EOF
 */
inline char getch() {
    struct termios previous, raw;

    if (tcgetattr(STDIN_FILENO, &previous) < 0) {
        return 0;
    }

    // Modify a COPY, so `previous` stays pristine for the restore below.
    raw = previous;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    std::cout.flush();

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0) {
        return 0;
    }

    char buf = 0;
    ssize_t n;
    do {
        n = read(STDIN_FILENO, &buf, 1);
    } while (n < 0 && errno == EINTR);  // retry if a signal interrupted us

    tcsetattr(STDIN_FILENO, TCSADRAIN, &previous);

    return (n == 1) ? buf : 0;
}

/*
 * getchWithTimeout() - Get a character with timeout for real-time updates
 * 
 * Parameters:
 *   - timeoutMs: Maximum time to wait in milliseconds
 * 
 * Returns: The character pressed, or 0 if timeout occurred
 */
inline char getchWithTimeout(int timeoutMs) {
    struct termios previous, raw;

    if (tcgetattr(STDIN_FILENO, &previous) < 0) {
        return 0;
    }

    raw = previous;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0) {
        return 0;
    }

    std::cout.flush();

    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(STDIN_FILENO, &readfds);

    struct timeval timeout;
    timeout.tv_sec  = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;

    char result = 0;
    int ready;
    do {
        ready = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
    } while (ready < 0 && errno == EINTR);

    if (ready > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
        ssize_t n;
        do {
            n = read(STDIN_FILENO, &result, 1);
        } while (n < 0 && errno == EINTR);
        if (n != 1) result = 0;
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &previous);

    return result;
}

/*
 * clearScreen() - Clear the terminal screen and move cursor to top-left
 */
inline void clearScreen() {
    std::cout << "\033[2J\033[1;1H";
}

/*
 * printLine() - Print a horizontal line for visual separation
 */
inline void printLine(char ch = '=', int length = 50, const std::string& color = Color::CYAN) {
    std::cout << color;
    for (int i = 0; i < length; i++) {
        std::cout << ch;
    }
    std::cout << Color::RESET << std::endl;
}

/*
 * printCentered() - Print text centered within a given width
 *
 * Centers on terminal COLUMNS, not bytes, so titles containing emoji or card
 * suits land where they should. See core/text_width.h.
 */
inline void printCentered(const std::string& text, int width = 50, const std::string& color = "") {
    const int padding = text::padTo(text, width) / 2;
    std::cout << std::string(padding, ' ') << color << text << Color::RESET << std::endl;
}

/*
 * printPadded() - Print text followed by spaces out to a column count
 *
 * Use instead of `for (i = 0; i < N - str.length(); i++) cout << ' '`, which
 * mis-aligns on any non-ASCII string.
 */
inline void printPadded(const std::string& text, int columns) {
    std::cout << text << std::string(text::padTo(text, columns), ' ');
}

/*
 * handleArrowKeys() - Process arrow key escape sequences
 * 
 * Arrow keys send: ESC [ A (up), ESC [ B (down), ESC [ C (right), ESC [ D (left)
 * 
 * Parameters:
 *   - firstChar: The first character read (should be ESC = 27)
 * 
 * Returns: 'w' for up, 's' for down, 'a' for left, 'd' for right, or 0 if not an arrow
 */
inline char handleArrowKeys(char firstChar) {
    if (firstChar != 27) return 0;  // Not ESC
    
    char seq1 = getchWithTimeout(50);
    if (seq1 != '[') return 0;
    
    char seq2 = getchWithTimeout(50);
    switch (seq2) {
        case 'A': return 'w';  // Up arrow
        case 'B': return 's';  // Down arrow
        case 'C': return 'd';  // Right arrow
        case 'D': return 'a';  // Left arrow
        default: return 0;
    }
}

/*
 * getMenuInput() - Get input for menu navigation with arrow/WASD support
 * 
 * Parameters:
 *   - timeoutMs: Timeout in milliseconds (0 for blocking)
 * 
 * Returns: 'w'=up, 's'=down, 'a'=left, 'd'=right, '\n'=select, 'q'=quit, or the raw char
 */
inline char getMenuInput(int timeoutMs = 1000) {
    char choice = (timeoutMs > 0) ? getchWithTimeout(timeoutMs) : getch();
    
    if (choice == 0) return 0;  // Timeout
    
    // Handle arrow keys
    if (choice == 27) {
        char arrow = handleArrowKeys(choice);
        if (arrow != 0) return arrow;
        return 0;  // Just ESC pressed
    }
    
    // Normalize WASD to lowercase
    if (choice == 'W') choice = 'w';
    if (choice == 'A') choice = 'a';
    if (choice == 'S') choice = 's';
    if (choice == 'D') choice = 'd';
    
    return choice;
}

#endif // UTILS_H
