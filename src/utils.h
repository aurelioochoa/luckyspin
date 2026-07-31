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
#include "colors.h"

/*
 * getch() - Get a single character from the terminal without waiting for Enter
 * 
 * This function modifies terminal settings temporarily to:
 * 1. Disable canonical mode (line buffering)
 * 2. Disable echo
 * 
 * Returns: The character that was pressed
 */
inline char getch() {
    char buf = 0;
    struct termios old;
    memset(&old, 0, sizeof(old));
    
    fflush(stdout);
    
    if (tcgetattr(0, &old) < 0) {
        perror("tcgetattr()");
    }
    
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    
    if (tcsetattr(0, TCSANOW, &old) < 0) {
        perror("tcsetattr ICANON");
    }
    
    if (read(0, &buf, 1) < 0) {
        perror("read()");
    }
    
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    
    if (tcsetattr(0, TCSADRAIN, &old) < 0) {
        perror("tcsetattr ~ICANON");
    }
    
    return buf;
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
    struct termios old, current;
    memset(&old, 0, sizeof(old));
    
    tcgetattr(0, &old);
    current = old;
    current.c_lflag &= ~ICANON;
    current.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &current);
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(0, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = timeoutMs / 1000;
    timeout.tv_usec = (timeoutMs % 1000) * 1000;
    
    char result = 0;
    int ready = select(1, &readfds, NULL, NULL, &timeout);
    
    if (ready > 0 && FD_ISSET(0, &readfds)) {
        read(0, &result, 1);
    }
    
    tcsetattr(0, TCSANOW, &old);
    
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
 */
inline void printCentered(const std::string& text, int width = 50, const std::string& color = "") {
    int padding = (width - text.length()) / 2;
    if (padding < 0) padding = 0;
    std::cout << std::string(padding, ' ') << color << text << Color::RESET << std::endl;
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
