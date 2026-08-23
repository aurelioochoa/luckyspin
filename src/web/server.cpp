#include <string>
#include <functional>
#include <stdexcept>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "server.h"

namespace web {

namespace {

volatile sig_atomic_t g_stop = 0;

extern "C" void stopHandler(int) { g_stop = 1; }

/*
 * readRequest() - Pull one request off the socket.
 *
 * Headers end at CRLFCRLF; a Content-Length then tells us how much body to
 * expect. The receive timeout turns half-open connections into clean
 * failures instead of a stuck server.
 */
bool readRequest(int fd, Request& req) {
    std::string raw;
    char buf[4096];
    size_t headerEnd = std::string::npos;

    while (raw.size() < (1u << 20)) {   // 1 MiB cap: plenty for this API
        if (headerEnd == std::string::npos) {
            headerEnd = raw.find("\r\n\r\n");
            if (headerEnd != std::string::npos) break;
        }
        const ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) return false;
        raw.append(buf, (size_t)n);
    }
    if (headerEnd == std::string::npos) {
        headerEnd = raw.find("\r\n\r\n");
        if (headerEnd == std::string::npos) return false;
    }

    // Request line: METHOD SP PATH SP VERSION
    const std::string headers = raw.substr(0, headerEnd);
    const size_t lineEnd = headers.find("\r\n");
    const std::string requestLine =
        (lineEnd == std::string::npos) ? headers : headers.substr(0, lineEnd);

    size_t sp1 = requestLine.find(' ');
    size_t sp2 = (sp1 == std::string::npos) ? std::string::npos
                                            : requestLine.find(' ', sp1 + 1);
    if (sp1 == std::string::npos || sp2 == std::string::npos) return false;
    req.method = requestLine.substr(0, sp1);
    req.path = requestLine.substr(sp1 + 1, sp2 - sp1 - 1);

    // Only the methods and path shapes this API speaks; anything else is a
    // malformed request rather than a mystery route for the handler.
    if ((req.method != "GET" && req.method != "POST" && req.method != "HEAD")
        || req.path.empty() || req.path[0] != '/') {
        return false;
    }

    // Strip any query string; the API does not use one.
    const size_t q = req.path.find('?');
    if (q != std::string::npos) req.path.resize(q);

    // Content-Length for the body (if any).
    size_t contentLength = 0;
    size_t pos = headers.find("Content-Length:");
    if (pos == std::string::npos) pos = headers.find("content-length:");
    if (pos != std::string::npos) {
        contentLength = (size_t)std::strtoul(headers.c_str() + pos + 15, nullptr, 10);
    }
    if (contentLength > (1u << 20)) return false;

    std::string body = raw.substr(headerEnd + 4);
    while (body.size() < contentLength) {
        const ssize_t n = ::recv(fd, buf, sizeof(buf), 0);
        if (n <= 0) break;
        body.append(buf, (size_t)n);
    }
    body.resize(std::min(contentLength, body.size()));
    req.body = body;
    return true;
}

void writeAll(int fd, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        const ssize_t n = ::send(fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n <= 0) return;
        sent += (size_t)n;
    }
}

const char* statusText(int status) {
    switch (status) {
        case 200: return "OK";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        default:  return "Server Error";
    }
}

void sendResponse(int fd, const Response& res) {
    std::ostringstream head;
    head << "HTTP/1.1 " << res.status << " " << statusText(res.status) << "\r\n"
         << "Content-Type: " << res.contentType << "; charset=utf-8\r\n"
         << "Content-Length: " << res.body.size() << "\r\n"
         << "Cache-Control: no-store\r\n"
         << "Connection: close\r\n"
         << "\r\n";
    writeAll(fd, head.str());
    writeAll(fd, res.body);
}

} // namespace

void serveConnection(int fd, const Handler& handler) {
    // 2s receive timeout: half-open connections fail cleanly instead of
    // wedging the sequential accept loop.
    struct timeval tv{2, 0};
    ::setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    Request req;
    Response res;
    if (!readRequest(fd, req)) {
        res.status = 400;
        res.contentType = "application/json";
        res.body = "{\"ok\":false,\"message\":\"malformed request\"}";
    } else {
        res = handler(req);
    }
    sendResponse(fd, res);
    ::close(fd);
}

/*
 * installStopHandler() - Catch INT/TERM without SA_RESTART.
 *
 * signal() on Linux/glibc implies SA_RESTART: accept() would silently resume
 * after the handler ran and the stop flag would sit unchecked until some
 * future connection arrived. Plain sigaction() with zero flags lets accept()
 * fail with EINTR so the loop can notice g_stop immediately.
 */
void installStopHandler(int sig) {
    struct sigaction sa{};
    sa.sa_handler = stopHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;   // deliberately no SA_RESTART
    ::sigaction(sig, &sa, nullptr);
}

int serveLoop(int port, const Handler& handler,
              const std::function<void()>& onShutdown,
              const std::function<void(int)>& onStart) {
    g_stop = 0;   // a previous loop in this process may have set it
    installStopHandler(SIGINT);
    installStopHandler(SIGTERM);
    ::signal(SIGPIPE, SIG_IGN);

    const int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) throw std::runtime_error("socket() failed");

    int reuse = 1;
    ::setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);   // never exposed to LAN
    addr.sin_port = 0;

    int bound = -1;
    for (int attempt = 0; attempt <= 20; attempt++) {
        addr.sin_port = htons((unsigned short)(port + attempt));
        if (::bind(listenFd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
            bound = port + attempt;
            break;
        }
        if (attempt == 20 || errno != EADDRINUSE) break;
    }
    if (bound < 0) {
        ::close(listenFd);
        throw std::runtime_error("could not bind a local port");
    }
    if (::listen(listenFd, 8) != 0) {
        ::close(listenFd);
        throw std::runtime_error("listen() failed");
    }

    if (onStart) onStart(bound);

    while (!g_stop) {
        struct sockaddr_in peer{};
        socklen_t peerLen = sizeof(peer);
        const int fd = ::accept(listenFd, (struct sockaddr*)&peer, &peerLen);
        if (fd < 0) {
            if (errno == EINTR) continue;   // signal may have hit accept()
            continue;
        }

        serveConnection(fd, handler);   // reads, dispatches, responds, closes
    }

    ::close(listenFd);
    if (onShutdown) onShutdown();
    return bound;
}

} // namespace web
