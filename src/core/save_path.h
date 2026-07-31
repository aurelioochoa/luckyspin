/*
 * save_path.h - Where the save file lives
 *
 * The game used to write "lucky_spin_save.txt" relative to the current working
 * directory. Since `make install` puts the binary on $PATH, running it from two
 * different directories silently produced two unrelated save files.
 *
 * Saves now go to the XDG base directory:
 *   $XDG_DATA_HOME/luckyspin/   (if XDG_DATA_HOME is set)
 *   ~/.local/share/luckyspin/   (otherwise)
 *
 * A legacy save in the current directory is migrated on first run.
 */

#ifndef CORE_SAVE_PATH_H
#define CORE_SAVE_PATH_H

#include <string>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <sys/types.h>
#include <cerrno>

namespace savepath {

inline std::string joinPath(const std::string& a, const std::string& b) {
    if (a.empty()) return b;
    if (a[a.size() - 1] == '/') return a + b;
    return a + "/" + b;
}

/*
 * dataDir() - Base directory for this game's data, without trailing slash.
 *
 * Returns an empty string if neither XDG_DATA_HOME nor HOME is set, in which
 * case the caller should fall back to the current directory.
 */
inline std::string dataDir() {
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (xdg && *xdg) {
        return joinPath(xdg, "luckyspin");
    }
    const char* home = std::getenv("HOME");
    if (home && *home) {
        return joinPath(joinPath(home, ".local/share"), "luckyspin");
    }
    return std::string();
}

/*
 * ensureDir() - Create a directory and any missing parents (like mkdir -p).
 *
 * Returns true if the directory exists (or was created) afterwards.
 */
inline bool ensureDir(const std::string& path) {
    if (path.empty()) return false;

    for (std::size_t i = 1; i <= path.size(); ++i) {
        if (i == path.size() || path[i] == '/') {
            const std::string part = path.substr(0, i);
            if (::mkdir(part.c_str(), 0700) != 0 && errno != EEXIST) {
                return false;
            }
        }
    }

    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode);
}

inline bool fileExists(const std::string& path) {
    struct stat st;
    return ::stat(path.c_str(), &st) == 0 && S_ISREG(st.st_mode);
}

/*
 * resolve() - Full path to a save file, creating the directory if needed.
 *
 * Falls back to the bare filename (current directory) if the data directory
 * cannot be created, so the game still saves rather than silently failing.
 */
inline std::string resolve(const std::string& filename) {
    const std::string dir = dataDir();
    if (!dir.empty() && ensureDir(dir)) {
        return joinPath(dir, filename);
    }
    return filename;
}

/*
 * migrateLegacy() - Move a pre-XDG save from the current directory.
 *
 * Only acts when a legacy file exists and no save is present at the new
 * location, so it can never clobber newer progress. Returns true if a file
 * was migrated.
 */
inline bool migrateLegacy(const std::string& filename, const std::string& resolvedPath) {
    if (resolvedPath == filename) return false;   // not using a data dir
    if (!fileExists(filename)) return false;      // nothing to migrate
    if (fileExists(resolvedPath)) return false;   // never overwrite a real save

    return std::rename(filename.c_str(), resolvedPath.c_str()) == 0;
}

} // namespace savepath

#endif // CORE_SAVE_PATH_H
