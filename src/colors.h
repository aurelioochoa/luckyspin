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
#include <cstdlib>

namespace Color {
    inline std::string code(const char* value) {
        return std::getenv("NO_COLOR") ? "" : value;
    }
    // Reset - clears all formatting
    const std::string RESET   = code("\033[0m");
    
    // Text styles
    const std::string BOLD    = code("\033[1m");
    const std::string DIM     = code("\033[2m");
    const std::string ITALIC  = code("\033[3m");
    const std::string UNDERLINE = code("\033[4m");
    const std::string BLINK   = code("\033[5m");
    const std::string REVERSE = code("\033[7m");
    
    // Foreground colors (text color)
    const std::string BLACK   = code("\033[30m");
    const std::string RED     = code("\033[31m");
    const std::string GREEN   = code("\033[32m");
    const std::string YELLOW  = code("\033[33m");
    const std::string BLUE    = code("\033[34m");
    const std::string MAGENTA = code("\033[35m");
    const std::string CYAN    = code("\033[36m");
    const std::string WHITE   = code("\033[37m");
    
    // Bright foreground colors
    const std::string BRIGHT_BLACK   = code("\033[90m");
    const std::string BRIGHT_RED     = code("\033[91m");
    const std::string BRIGHT_GREEN   = code("\033[92m");
    const std::string BRIGHT_YELLOW  = code("\033[93m");
    const std::string BRIGHT_BLUE    = code("\033[94m");
    const std::string BRIGHT_MAGENTA = code("\033[95m");
    const std::string BRIGHT_CYAN    = code("\033[96m");
    const std::string BRIGHT_WHITE   = code("\033[97m");
    
    // Background colors
    const std::string BG_BLACK   = code("\033[40m");
    const std::string BG_RED     = code("\033[41m");
    const std::string BG_GREEN   = code("\033[42m");
    const std::string BG_YELLOW  = code("\033[43m");
    const std::string BG_BLUE    = code("\033[44m");
    const std::string BG_MAGENTA = code("\033[45m");
    const std::string BG_CYAN    = code("\033[46m");
    const std::string BG_WHITE   = code("\033[47m");
    
    // Bright background colors
    const std::string BG_BRIGHT_BLACK   = code("\033[100m");
    const std::string BG_BRIGHT_RED     = code("\033[101m");
    const std::string BG_BRIGHT_GREEN   = code("\033[102m");
    const std::string BG_BRIGHT_YELLOW  = code("\033[103m");
    const std::string BG_BRIGHT_BLUE    = code("\033[104m");
    const std::string BG_BRIGHT_MAGENTA = code("\033[105m");
    const std::string BG_BRIGHT_CYAN    = code("\033[106m");
    const std::string BG_BRIGHT_WHITE   = code("\033[107m");
}

#endif // COLORS_H
