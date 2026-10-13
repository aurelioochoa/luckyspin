/*
 * ============================================================================
 * LUCKY SPIN CASINO - A Terminal-Based Incremental Gambling Game
 * ============================================================================
 *
 * Two frontends share one headless engine:
 *   ./gambling_game            classic terminal UI
 *   ./gambling_game --gui      local web GUI (http://127.0.0.1:<port>)
 *
 * Optional flags:
 *   --port N    port for --gui (default 8787; busy ports count upward)
 *   --no-open   do not launch a browser automatically
 */

#include <iostream>
#include <string>
#include <cstdlib>
#include <cerrno>
#include <unistd.h>

#include "gambling_game.h"
#include "utils.h" // terminal::init()/restore()
#include "web/server.h"
#include "web/api.h"

namespace {

int portFlag = 8787;
bool openBrowser = true;

bool validPort(const std::string& value) {
    if (value.empty() || value.find_first_not_of("0123456789") != std::string::npos) return false;
    char* end = nullptr;
    errno = 0;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (errno || *end || parsed < 1 || parsed > 65535) return false;
    portFlag = (int)parsed;
    return true;
}

int parseFlags(int argc, char** argv, bool& guiMode) {
    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--gui") {
            guiMode = true;
        } else if (arg == "--no-open") {
            openBrowser = false;
        } else if (arg == "--port" && i + 1 < argc) {
            if (!validPort(argv[++i])) {
                std::cerr << "Port must be an integer from 1 to 65535.\n";
                return 2;
            }
        } else if (arg.rfind("--port=", 0) == 0) {
            if (!validPort(arg.substr(7))) {
                std::cerr << "Port must be an integer from 1 to 65535.\n";
                return 2;
            }
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: gambling_game [--gui] [--port N] [--no-open]\n";
            return 1;   // help, caller exits cleanly
        } else {
            std::cerr << "Unknown flag: " << arg << " (try --help)\n";
            return 2;
        }
    }
    return 0;
}

void openInBrowser(const std::string& url) {
    const pid_t pid = ::fork();
    if (pid == 0) {
        ::execlp("xdg-open", "xdg-open", url.c_str(), (char*)nullptr);
        _exit(0);   // xdg-open missing or failed: not an error worth reporting
    }
}

} // namespace

int main(int argc, char** argv) {
    bool guiMode = false;
    const int flags = parseFlags(argc, argv, guiMode);
    if (flags) return flags == 1 ? 0 : 2;
    if (!guiMode && !isatty(STDIN_FILENO)) {
        std::cerr << "The terminal game needs an interactive terminal. Use --gui --no-open for the browser interface.\n";
        return 2;
    }

    try {
        GamblingGame game;

        if (!guiMode) {
            /*
             * Snapshot terminal settings BEFORE anything switches to raw mode
             * and restore them on exit or fatal signal.
             */
            terminal::init();
            game.loadGame();
            game.play();
            return 0;
        }

        // Web GUI: no raw-terminal tricks; offline earnings apply silently.
        game.setHeadless(true);
        game.loadGame();

        std::cout <<
            "\n  \xF0\x9F\x8E\xB0 Lucky Spin Casino - Web GUI\n"
            "  --------------------------------\n";

        const int bound = web::serveLoop(
            portFlag,
            [&game](const web::Request& req) { return api::handle(game, req); },
            [&game]() { game.saveGame(); },
            [](int port) {
                const std::string url = "http://127.0.0.1:" + std::to_string(port) + "/";
                std::cout << "  Serving:  " << url << "\n"
                          << "  Stop:     Ctrl+C (progress is saved)\n\n"
                          << std::flush;
                if (openBrowser) openInBrowser(url);
            });

        (void)bound;
        return 0;
    }
    catch (const std::exception& e) {
        if (guiMode) {
            std::cerr << "\nLucky Spin GUI failed to start: " << e.what() << "\n";
        } else {
            terminal::restore();
            std::cerr << "\nLucky Spin hit an unexpected error: " << e.what() << "\n";
        }
        return 1;
    }
    catch (...) {
        if (!guiMode) terminal::restore();
        std::cerr << "\nLucky Spin hit an unexpected error.\n";
        return 1;
    }
}
