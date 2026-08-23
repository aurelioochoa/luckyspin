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
// PERSISTENCE (atomic save/load, safe parsing, checksum)
// ==========================================================================

// ============================================================================
// STATIC MEMBER INITIALIZATION
// ============================================================================
/*
 * Static member variables must be defined outside the class.
 * This is because static members are shared across all instances and need
 * a single definition in memory.
 */
const string GamblingGame::SAVE_FILENAME = "lucky_spin_save.txt";

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
bool GamblingGame::saveGame() {
    /*
     * Written to a temp file first, then renamed into place. rename() is
     * atomic within a filesystem, so a crash or full disk mid-write leaves
     * the previous save intact instead of a half-written one.
     */
    const string temp = tempPath();

    // Built in memory first so the checksum below can cover every byte
    // of the body exactly as it will land on disk.
    ostringstream body;
    
    // Write save file header with version
    body << "LUCKY_SPIN_SAVE_V1" << endl;
    body << "timestamp=" << time(0) << endl;
    
    // Save player stats
    body << "money=" << fixed << setprecision(2) << money << endl;
    body << "betAmount=" << fixed << setprecision(2) << betAmount << endl;
    body << "round=" << round << endl;
    body << "totalWins=" << totalWins << endl;
    body << "totalLosses=" << totalLosses << endl;
    body << "peakMoney=" << fixed << setprecision(2) << peakMoney << endl;
    body << "lifetimeEarnings=" << fixed << setprecision(2) << lifetimeEarnings << endl;
    
    // Save generator owned counts
    // Format: gen_0=5, gen_1=3, etc.
    for (size_t i = 0; i < generators.size(); i++) {
        body << "gen_" << i << "=" << generators[i].getOwned() << endl;
    }
    
    // Save upgrade levels
    // Format: upg_0=2, upg_1=1, etc.
    for (size_t i = 0; i < upgrades.size(); i++) {
        body << "upg_" << i << "=" << upgrades[i].getCurrentLevel() << endl;
    }
    
    // Save prestige data (Sprint 4)
    body << "prestigeLevel=" << prestigeLevel << endl;
    body << "prestigeMultiplier=" << fixed << setprecision(2) << prestigeMultiplier << endl;
    body << "totalLifetimeEarnings=" << fixed << setprecision(2) << totalLifetimeEarnings << endl;
    
    // Save achievements (Sprint 4)
    body << "achFirstWin=" << achFirstWin << endl;
    body << "achHighRoller=" << achHighRoller << endl;
    body << "achMillionaire=" << achMillionaire << endl;
    body << "achLuckyStreak=" << achLuckyStreak << endl;
    body << "achGenerator10=" << achGenerator10 << endl;
    body << "achAllUpgrades=" << achAllUpgrades << endl;
    body << "achPrestige1=" << achPrestige1 << endl;
    body << "achPrestige5=" << achPrestige5 << endl;
    body << "achPlinkoJackpot=" << achPlinkoJackpot << endl;
    body << "currentWinStreak=" << currentWinStreak << endl;
    
    // Save Phase B data
    body << "achievementMultiplier=" << fixed << setprecision(4) << achievementMultiplier << endl;
    body << "prestigeStars=" << prestigeStars << endl;
    body << "perkStartingMoney=" << perkStartingMoney << endl;
    body << "perkGeneratorDiscount=" << perkGeneratorDiscount << endl;
    body << "perkOfflineCap=" << perkOfflineCap << endl;
    body << "perkPassiveBoost=" << perkPassiveBoost << endl;
    
    // Save Phase C data
    body << "specializationId=" << specializationId << endl;
    body << "interestUnlocked=" << interestUnlocked << endl;
    body << "interestRate=" << fixed << setprecision(6) << interestRate << endl;
    for (int i = 0; i < 10; i++) {
        body << "milestone_" << i << "=" << milestoneUnlocked[i] << endl;
    }
    
    // Integrity trailer: FNV-1a over every byte written above. The loader
    // verifies it and warns on mismatch, but still loads (warn-only).
    const string content = body.str();
    body << "checksum=" << checksum::toHex(checksum::fnv1a(content)) << endl;

    ofstream file(temp.c_str());
    if (!file.is_open()) {
        cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to save game!" << Color::RESET;
        return false;
    }
    file << body.str();

    // Check for write errors (e.g. disk full) BEFORE replacing the old save.
    file.flush();
    const bool writeOk = file.good();
    file.close();
    
    if (!writeOk) {
        std::remove(temp.c_str());
        cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to write save data!" << Color::RESET;
        return false;
    }
    
    // Keep the previous save as a backup, then swap the new one in.
    if (savepath::fileExists(savePath())) {
        std::rename(savePath().c_str(), backupPath().c_str());
    }
    if (std::rename(temp.c_str(), savePath().c_str()) != 0) {
        std::remove(temp.c_str());
        cout << "\n  " << Color::RED << UI::CROSS_X << " Failed to finalize save!" << Color::RESET;
        return false;
    }
    
    lastSaveTime = time(0);
    
    return true;
}

// ========================================================================
// SAFE SAVE-FILE PARSING
// ========================================================================
/*
 * stod()/stoi() throw on malformed input. A save file is user-writable and
 * can be truncated by a crash, so a bad value must never take the process
 * down. These helpers report failure instead and leave the target alone.
 */
bool GamblingGame::parseDouble(const string& text, double& out) {
    try {
        size_t consumed = 0;
        const double value = std::stod(text, &consumed);
        if (consumed == 0) return false;
        if (!(value == value)) return false;              // reject NaN
        if (value > 1e300 || value < -1e300) return false; // reject inf
        out = value;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool GamblingGame::parseInt(const string& text, int& out) {
    try {
        size_t consumed = 0;
        const int value = std::stoi(text, &consumed);
        if (consumed == 0) return false;
        out = value;
        return true;
    } catch (const std::exception&) {
        return false;   // covers both invalid_argument and out_of_range
    }
}

bool GamblingGame::parseLong(const string& text, long& out) {
    try {
        size_t consumed = 0;
        const long value = std::stol(text, &consumed);
        if (consumed == 0) return false;
        out = value;
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool GamblingGame::parseBool(const string& text, bool& out) {
    int value = 0;
    if (!parseInt(text, value)) return false;
    out = (value != 0);
    return true;
}

// Non-negative doubles (money, earnings) clamped at 0.
bool GamblingGame::parseMoney(const string& text, double& out) {
    double value = 0.0;
    if (!parseDouble(text, value)) return false;
    out = (value < 0.0) ? 0.0 : value;
    return true;
}

// Counts must be >= 0 and within a sane bound.
bool GamblingGame::parseCount(const string& text, int& out, int maxValue) {
    int value = 0;
    if (!parseInt(text, value)) return false;
    if (value < 0) value = 0;
    if (value > maxValue) value = maxValue;
    out = value;
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
bool GamblingGame::loadGame() {
    // Bring a pre-XDG save in the current directory forward, once.
    savepath::migrateLegacy(SAVE_FILENAME, savePath());
    
    time_t savedTimestamp = 0;
    
    // Try the real save; if it is missing or unreadable, fall back to the
    // backup written by the previous successful save.
    bool loaded = loadFrom(savePath(), savedTimestamp);
    if (!loaded) {
        loaded = loadFrom(backupPath(), savedTimestamp);
        if (loaded) {
            cout << "\n  " << Color::YELLOW << UI::CROSS_X
                 << " Save file was unreadable - restored from backup."
                 << Color::RESET << "\n";
            usleep(1500000);
        }
    }
    
    if (!loaded) {
        // No usable save. Normal for new players.
        return false;
    }
    
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
 * loadFrom() - Parse one save file into the current game state.
 *
 * Every value goes through a parse helper that cannot throw. An
 * unrecognized key or an unparseable value is skipped, leaving that field
 * at its constructor default, so a partially corrupt file still restores
 * as much progress as possible instead of crashing the game.
 *
 * Returns: true if the file existed and had a valid header
 */
bool GamblingGame::loadFrom(const string& path, time_t& savedTimestamp) {
    ifstream file(path.c_str());

    if (!file.is_open()) {
        return false;
    }

    // Read raw bytes so the checksum can be verified over exactly what
    // saveGame() hashed, newline-for-newline.
    ostringstream raw;
    raw << file.rdbuf();
    const string content = raw.str();
    file.close();

    istringstream lines(content);
    string line;

    // Read and verify header
    if (!getline(lines, line) || line != "LUCKY_SPIN_SAVE_V1") {
        return false;  // Invalid or incompatible save file
    }

    // Verify the integrity trailer if present. Pre-checksum saves have no
    // trailer and load silently; a mismatch warns but still loads, so a
    // corrupt or hand-edited file never locks the player out.
    const string trailerTag = "\nchecksum=";
    const size_t trailerPos = content.rfind(trailerTag);
    if (trailerPos != string::npos) {
        string stored = content.substr(trailerPos + trailerTag.size());
        while (!stored.empty() &&
               (stored[stored.size() - 1] == '\n' || stored[stored.size() - 1] == '\r')) {
            stored.erase(stored.size() - 1);
        }
        uint64_t expected = 0;
        const string hashedBody = content.substr(0, trailerPos + 1);
        if (!checksum::fromHex(stored, expected) ||
            checksum::fnv1a(hashedBody) != expected) {
            cout << "\n  " << Color::YELLOW << UI::CROSS_X
                 << " Save file checksum mismatch - it may be corrupt or edited."
                 << " Loading anyway." << Color::RESET << "\n";
            usleep(1500000);
        }
    }

    // Parse each line
    while (getline(lines, line)) {
        // Strip a trailing CR so files touched on Windows still load
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        
        // Find the '=' separator
        size_t eqPos = line.find('=');
        if (eqPos == string::npos) continue;  // Skip invalid lines
        
        // Split into key and value
        const string key = line.substr(0, eqPos);
        const string value = line.substr(eqPos + 1);
        
        if (key == "timestamp") {
            long parsed = 0;
            if (parseLong(value, parsed) && parsed > 0) {
                savedTimestamp = static_cast<time_t>(parsed);
            }
        } else if (key == "money") {
            parseMoney(value, money);
        } else if (key == "betAmount") {
            parseMoney(value, betAmount);
        } else if (key == "round") {
            parseCount(value, round, INT_MAX - 1);
        } else if (key == "totalWins") {
            parseCount(value, totalWins, INT_MAX - 1);
        } else if (key == "totalLosses") {
            parseCount(value, totalLosses, INT_MAX - 1);
        } else if (key == "peakMoney") {
            parseMoney(value, peakMoney);
        } else if (key == "lifetimeEarnings") {
            parseMoney(value, lifetimeEarnings);
        } else if (key.compare(0, 4, "gen_") == 0) {
            // Generator data: gen_0, gen_1, etc.
            int index = -1;
            int owned = 0;
            if (parseInt(key.substr(4), index) &&
                index >= 0 && index < (int)generators.size() &&
                parseCount(value, owned, Generator::MAX_OWNED)) {
                generators[index].setOwned(owned);
            }
        } else if (key.compare(0, 4, "upg_") == 0) {
            // Upgrade data: upg_0, upg_1, etc.
            int index = -1;
            int level = 0;
            if (parseInt(key.substr(4), index) &&
                index >= 0 && index < (int)upgrades.size() &&
                parseCount(value, level, upgrades[index].getMaxLevel())) {
                upgrades[index].setLevel(level);
            }
        }
        // Prestige data (Sprint 4)
        else if (key == "prestigeLevel") {
            parseCount(value, prestigeLevel, 100000);
        } else if (key == "prestigeMultiplier") {
            double parsed = 1.0;
            if (parseDouble(value, parsed) && parsed >= 1.0) prestigeMultiplier = parsed;
        } else if (key == "totalLifetimeEarnings") {
            parseMoney(value, totalLifetimeEarnings);
        }
        // Achievement data (Sprint 4)
        else if (key == "achFirstWin") {
            parseBool(value, achFirstWin);
        } else if (key == "achHighRoller") {
            parseBool(value, achHighRoller);
        } else if (key == "achMillionaire") {
            parseBool(value, achMillionaire);
        } else if (key == "achLuckyStreak") {
            parseBool(value, achLuckyStreak);
        } else if (key == "achGenerator10") {
            parseBool(value, achGenerator10);
        } else if (key == "achAllUpgrades") {
            parseBool(value, achAllUpgrades);
        } else if (key == "achPrestige1") {
            parseBool(value, achPrestige1);
        } else if (key == "achPrestige5") {
            parseBool(value, achPrestige5);
        } else if (key == "achPlinkoJackpot") {
            parseBool(value, achPlinkoJackpot);
        } else if (key == "currentWinStreak") {
            parseCount(value, currentWinStreak, INT_MAX - 1);
        }
        // Phase B data
        else if (key == "achievementMultiplier") {
            double parsed = 1.0;
            if (parseDouble(value, parsed) && parsed >= 1.0) achievementMultiplier = parsed;
        } else if (key == "prestigeStars") {
            parseCount(value, prestigeStars, INT_MAX - 1);
        } else if (key == "perkStartingMoney") {
            parseCount(value, perkStartingMoney, 10);
        } else if (key == "perkGeneratorDiscount") {
            parseCount(value, perkGeneratorDiscount, 10);
        } else if (key == "perkOfflineCap") {
            parseCount(value, perkOfflineCap, 5);
        } else if (key == "perkPassiveBoost") {
            parseCount(value, perkPassiveBoost, 20);
        }
        // Phase C data
        else if (key == "specializationId") {
            // 5 = Plinko Pro; clamping to 4 here silently downgraded players
            parseCount(value, specializationId, 5);
        } else if (key == "interestUnlocked") {
            parseBool(value, interestUnlocked);
        } else if (key == "interestRate") {
            double parsed = 0.0;
            if (parseDouble(value, parsed) && parsed >= 0.0) interestRate = parsed;
        } else if (key.compare(0, 10, "milestone_") == 0) {
            int index = -1;
            if (parseInt(key.substr(10), index) && index >= 0 && index < 10) {
                parseBool(value, milestoneUnlocked[index]);
            }
        }
    }
    
    return true;
}

/*
 * autoSave() - Automatically save the game if enough time has passed
 * 
 * Called periodically in the game loop. Saves every 30 seconds.
 */
void GamblingGame::autoSave() {
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
void GamblingGame::deleteSaveFile() {
    remove(savePath().c_str());
    remove(backupPath().c_str());
    remove(tempPath().c_str());
}
