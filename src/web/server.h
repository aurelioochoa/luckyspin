/*
 * server.h - Tiny embedded HTTP/1.1 server for the local web GUI.
 *
 * Scope is deliberately narrow: one player, localhost only, two routes.
 * A sequential accept() loop with SO_RCVTIMEO keeps the whole thing about
 * 150 lines and free of threading hazards against the game state.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <string>
#include <functional>

namespace web {

struct Request {
    std::string method;   // "GET" / "POST"
    std::string path;     // "/api/state"
    std::string body;
};

struct Response {
    int status = 200;
    std::string contentType = "application/json";
    std::string body;
};

using Handler = std::function<Response(const Request&)>;

/*
 * serveConnection() - Handle exactly one connection on an already-accepted
 * socket: read one request, dispatch it through the handler, write the
 * response, close. Exposed so tests can drive the wire format directly
 * over a socketpair without binding ports or spawning threads.
 */
void serveConnection(int fd, const Handler& handler);

/*
 * serveLoop() - Bind 127.0.0.1:port and handle requests forever.
 *
 * Returns the actual bound port (useful when the caller passed 0 for an
 * OS-assigned port). Calls onStart(port) once the socket is listening - a
 * good place to print the URL or launch a browser - and onShutdown()
 * exactly when SIGINT/SIGTERM ends the accept loop; that callback should
 * persist state.
 * Throws std::runtime_error if no port in [port, port+20] can be bound.
 */
int serveLoop(int port, const Handler& handler,
              const std::function<void()>& onShutdown,
              const std::function<void(int)>& onStart = {});

} // namespace web

#endif // WEB_SERVER_H
