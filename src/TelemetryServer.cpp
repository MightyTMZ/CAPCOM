#include "capcom/TelemetryServer.h"

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

class WinSockSession {
public:
    WinSockSession() {
        WSADATA data{};
        if (WSAStartup(MAKEWORD(2, 2), &data) != 0) {
            throw std::runtime_error("Could not initialize WinSock");
        }
    }

    ~WinSockSession() {
        WSACleanup();
    }
};

class SocketHandle {
public:
    explicit SocketHandle(SOCKET socket = INVALID_SOCKET) : socket_(socket) {}

    ~SocketHandle() {
        if (socket_ != INVALID_SOCKET) {
            closesocket(socket_);
        }
    }

    SocketHandle(const SocketHandle&) = delete;
    SocketHandle& operator=(const SocketHandle&) = delete;

    SOCKET get() const {
        return socket_;
    }

private:
    SOCKET socket_;
};

void sendAll(SOCKET client, const std::string& response) {
    size_t totalSent = 0;
    while (totalSent < response.size()) {
        const int sent = send(
            client,
            response.data() + totalSent,
            static_cast<int>(response.size() - totalSent),
            0);
        if (sent == SOCKET_ERROR) {
            throw std::runtime_error("Could not send HTTP response");
        }
        totalSent += static_cast<size_t>(sent);
    }
}

std::string makeResponse(
    const std::string& status,
    const std::string& contentType,
    const std::string& body) {
    return "HTTP/1.1 " + status + "\r\n"
           "Content-Type: " + contentType + "\r\n"
           "Content-Length: " + std::to_string(body.size()) + "\r\n"
           "Connection: close\r\n"
           "Cache-Control: no-store\r\n"
           "\r\n" + body;
}

}

namespace capcom {

TelemetryServer::TelemetryServer(
    uint16_t port,
    TelemetryProvider telemetryProvider)
    : port_(port), telemetryProvider_(std::move(telemetryProvider)) {}

void TelemetryServer::run() const {
    const WinSockSession winSock;
    const SocketHandle server(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (server.get() == INVALID_SOCKET) {
        throw std::runtime_error("Could not create server socket");
    }

    int reuseAddress = 1;
    setsockopt(
        server.get(),
        SOL_SOCKET,
        SO_REUSEADDR,
        reinterpret_cast<const char*>(&reuseAddress),
        sizeof(reuseAddress));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(port_);

    if (bind(
            server.get(),
            reinterpret_cast<const sockaddr*>(&address),
            sizeof(address)) == SOCKET_ERROR) {
        throw std::runtime_error(
            "Could not bind to port " + std::to_string(port_));
    }
    if (listen(server.get(), SOMAXCONN) == SOCKET_ERROR) {
        throw std::runtime_error("Could not listen for HTTP requests");
    }

    std::cout << "Telemetry endpoint: http://0.0.0.0:" << port_
              << "/telemetry\n";

    while (true) {
        const SocketHandle client(accept(server.get(), nullptr, nullptr));
        if (client.get() == INVALID_SOCKET) {
            std::cerr << "Could not accept HTTP connection\n";
            continue;
        }

        char buffer[4096]{};
        const int received = recv(
            client.get(),
            buffer,
            static_cast<int>(sizeof(buffer) - 1),
            0);
        if (received <= 0) {
            continue;
        }

        const std::string request(buffer, static_cast<size_t>(received));
        std::string response;

        if (request.starts_with("GET /telemetry ")) {
            const std::string body = telemetryProvider_();
            response = makeResponse("200 OK", "application/json", body);
        } else if (request.starts_with("GET ")) {
            response = makeResponse(
                "404 Not Found",
                "application/json",
                "{\"error\":\"Not found\"}");
        } else {
            response = makeResponse(
                "405 Method Not Allowed",
                "application/json",
                "{\"error\":\"Only GET is supported\"}");
        }

        try {
            sendAll(client.get(), response);
        } catch (const std::exception& error) {
            std::cerr << error.what() << '\n';
        }
    }
}

}
