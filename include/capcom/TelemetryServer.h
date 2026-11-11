#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace capcom {

class TelemetryServer {
public:
    using TelemetryProvider = std::function<std::string()>;

    TelemetryServer(uint16_t port, TelemetryProvider telemetryProvider);
    void run() const;

private:
    uint16_t port_;
    TelemetryProvider telemetryProvider_;
};

}
