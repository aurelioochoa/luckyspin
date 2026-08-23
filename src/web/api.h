/*
 * api.h - JSON API bridging the web GUI to the game engine.
 *
 * GET  /           -> embedded single-page frontend
 * GET  /api/state  -> full snapshot (also ticks income/events)
 * POST /api/action -> {"action": "...", ...}; response embeds fresh state
 */

#ifndef WEB_API_H
#define WEB_API_H

#include <string>

#include "../gambling_game.h"
#include "server.h"

namespace api {

// Full game snapshot as JSON. Ticks passive income/events first so polling
// alone keeps the idle economy running.
std::string stateJson(GamblingGame& game);

// Route one HTTP request.
web::Response handle(GamblingGame& game, const web::Request& req);

} // namespace api

#endif // WEB_API_H
