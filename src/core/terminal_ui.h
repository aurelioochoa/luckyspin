#ifndef CORE_TERMINAL_UI_H
#define CORE_TERMINAL_UI_H

#include <algorithm>
#include <string>
#include <vector>
#include <sstream>
#include "text_width.h"

namespace tui {

// Clip by display columns, preserving UTF-8 and complete ANSI sequences.
inline std::string fit(const std::string& value, int columns) {
    if (columns <= 0) return "";
    std::string out;
    std::size_t i = 0;
    while (i < value.size()) {
        const std::size_t start = i;
        if (value[i] == '\033') {
            ++i;
            if (i < value.size() && value[i] == '[') {
                ++i;
                while (i < value.size()) {
                    const unsigned char ch = value[i++];
                    if (ch >= 0x40 && ch <= 0x7e) break;
                }
            } else if (i < value.size()) ++i;
        } else {
            text::decodeUtf8(value, i);
        }
        const std::string next = out + value.substr(start, i - start);
        if (text::displayWidth(next) > columns) break;
        out = next;
    }
    return out + std::string(text::padTo(out, columns), ' ');
}

inline std::vector<std::string> wrap(const std::string& value, int columns) {
    std::vector<std::string> rows;
    std::istringstream words(value);
    std::string word, line;
    while (words >> word) {
        const std::string next = line.empty() ? word : line + " " + word;
        if (!line.empty() && text::displayWidth(next) > columns) {
            rows.push_back(line); line = word;
        } else line = next;
    }
    rows.push_back(line);
    return rows;
}

inline int dashboardWidth(int terminalColumns) {
    return std::max(20, std::min(92, terminalColumns - 4));
}

inline std::string shortcut(int index) {
    return index < 10 ? std::to_string(index) : " ";
}

} // namespace tui
#endif
