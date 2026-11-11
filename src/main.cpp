#include "capcom/TelemetryManager.h"
#include "capcom/TelemetryServer.h"
#include "capcom/collectors/CPUCollector.h"
#include "capcom/collectors/MemoryCollector.h"
#include "capcom/collectors/NetworkCollector.h"
#include "capcom/collectors/MouseCollector.h"
#include "capcom/collectors/KeyboardCollector.h"

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>

namespace {

std::string escapeJson(const std::string &value) {
    std::ostringstream output;

    for (const unsigned char character : value) {
        switch (character) {
        case '\\':
            output << "\\\\";
            break;
        case '"':
            output << "\\\"";
            break;
        case '\n':
            output << "\\n";
            break;
        case '\r':
            output << "\\r";
            break;
        case '\t':
            output << "\\t";
            break;
        default:
            if (character < 0x20) {
                output << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                       << static_cast<int>(character) << std::dec;
            } else {
                output << character;
            }
        }
    }

    return output.str();
}

std::string toJson(const capcom::Telemetry &telemetry) {
    std::ostringstream output;
    output << std::fixed << std::setprecision(1) << "{\"timestamp_ms\":" << telemetry.timestamp_ms
           << ",\"hostname\":\"" << escapeJson(telemetry.hostname) << '"'
           << ",\"cpu_percent\":" << telemetry.cpu_percent
           << ",\"memory_percent\":" << telemetry.memory_percent
           << ",\"network_rx_bytes_per_sec\":" << telemetry.network_rx_bytes_per_sec
           << ",\"network_tx_bytes_per_sec\":" << telemetry.network_tx_bytes_per_sec
           << ",\"mouse_clicks_last_2s\":" << telemetry.mouse_clicks_last_2s
           << ",\"keyboard_presses_last_2s\":" << telemetry.keyboard_presses_last_2s << '}';
    return output.str();
}

void printUsage() { std::cout << "Usage: capcom_sender [--port PORT]\n"; }

} // namespace

int main(int argc, char *argv[]) {
    uint16_t port = 8080;

    try {
        for (int index = 1; index < argc; ++index) {
            const std::string argument = argv[index];

            if (argument == "--port" && index + 1 < argc) {
                const int requestedPort = std::stoi(argv[++index]);
                if (requestedPort < 1 || requestedPort > 65535) {
                    throw std::runtime_error("Port must be between 1 and 65535");
                }
                port = static_cast<uint16_t>(requestedPort);
            } else if (argument == "--help") {
                printUsage();
                return 0;
            } else {
                throw std::runtime_error("Unknown or incomplete argument: " + argument);
            }
        }

        capcom::TelemetryManager manager;
        manager.addCollector(std::make_unique<capcom::CPUCollector>());
        manager.addCollector(std::make_unique<capcom::MemoryCollector>());
        manager.addCollector(std::make_unique<capcom::NetworkCollector>());
        manager.addCollector(std::make_unique<capcom::MouseCollector>());
        manager.addCollector(std::make_unique<capcom::KeyboardCollector>());
        manager.initialize();

        capcom::TelemetryServer server(port, [&manager] { return toJson(manager.collect()); });
        server.run();
    } catch (const std::exception &error) {
        std::cerr << "Error: " << error.what() << '\n';
        printUsage();
        return 1;
    }
}
