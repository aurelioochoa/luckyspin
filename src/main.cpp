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
#include <unistd.h>

#include "gambling_game.h"
#include "utils.h" // terminal::init()/restore()
#include "web/server.h"
#include "web/api.h"

namespace {

int portFlag = 8787;
bool openBrowser = true;

bool parseFlags(int argc, char** argv, bool& guiMode) {
    for (int i = 1; i < argc; i++) {
        const std::string arg = argv[i];
        if (arg == "--gui") {
            guiMode = true;
        } else if (arg == "--no-open") {
            openBrowser = false;
        } else if (arg == "--port" && i + 1 < argc) {
            portFlag = std::atoi(argv[++i]);
        } else if (arg.rfind("--port=", 0) == 0) {
            portFlag = std::atoi(arg.c_str() + 7);
        } else if (arg == "--help" || arg == "-h") {
            std::cout << "Usage: gambling_game [--gui] [--port N] [--no-open]\n";
            return false;   // caller exits cleanly
        } else {
            std::cerr << "Unknown flag: " << arg << " (try --help)\n";
            return false;
        }
    }
    return true;
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
    if (!parseFlags(argc, argv, guiMode)) return 0;

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
