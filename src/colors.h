/*
 * colors.h - ANSI color codes for terminal output
 * 
 * ANSI escape codes allow us to add colors and formatting to terminal output.
 * Format: \033[Xm where X is the color/style code
 * 
 * These codes work on most Unix/Linux terminals and modern Windows terminals.
 */

#ifndef COLORS_H
#define COLORS_H

#include <string>

namespace Color {
    // Reset - clears all formatting
    const std::string RESET   = "\033[0m";
    
    // Text styles
    const std::string BOLD    = "\033[1m";
    const std::string DIM     = "\033[2m";
    const std::string ITALIC  = "\033[3m";
    const std::string UNDERLINE = "\033[4m";
    const std::string BLINK   = "\033[5m";
    const std::string REVERSE = "\033[7m";
    
    // Foreground colors (text color)
    const std::string BLACK   = "\033[30m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string BLUE    = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN    = "\033[36m";
    const std::string WHITE   = "\033[37m";
    
    // Bright foreground colors
    const std::string BRIGHT_BLACK   = "\033[90m";
    const std::string BRIGHT_RED     = "\033[91m";
    const std::string BRIGHT_GREEN   = "\033[92m";
    const std::string BRIGHT_YELLOW  = "\033[93m";
    const std::string BRIGHT_BLUE    = "\033[94m";
    const std::string BRIGHT_MAGENTA = "\033[95m";
    const std::string BRIGHT_CYAN    = "\033[96m";
    const std::string BRIGHT_WHITE   = "\033[97m";
    
    // Background colors
    const std::string BG_BLACK   = "\033[40m";
    const std::string BG_RED     = "\033[41m";
    const std::string BG_GREEN   = "\033[42m";
    const std::string BG_YELLOW  = "\033[43m";
    const std::string BG_BLUE    = "\033[44m";
    const std::string BG_MAGENTA = "\033[45m";
    const std::string BG_CYAN    = "\033[46m";
    const std::string BG_WHITE   = "\033[47m";
    
    // Bright background colors
    const std::string BG_BRIGHT_BLACK   = "\033[100m";
    const std::string BG_BRIGHT_RED     = "\033[101m";
    const std::string BG_BRIGHT_GREEN   = "\033[102m";
    const std::string BG_BRIGHT_YELLOW  = "\033[103m";
    const std::string BG_BRIGHT_BLUE    = "\033[104m";
    const std::string BG_BRIGHT_MAGENTA = "\033[105m";
    const std::string BG_BRIGHT_CYAN    = "\033[106m";
    const std::string BG_BRIGHT_WHITE   = "\033[107m";
}

#endif // COLORS_H
