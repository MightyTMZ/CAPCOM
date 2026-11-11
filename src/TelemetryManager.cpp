#include "capcom/TelemetryManager.h"

#include <chrono>
#include <utility>

#include <Windows.h>

namespace capcom {

void TelemetryManager::addCollector(std::unique_ptr<ICollector> collector) {
    collectors_.push_back(std::move(collector));
}

void TelemetryManager::initialize() {
    for (auto& collector : collectors_) {
        collector->initialize();
    }
}

Telemetry TelemetryManager::collect() {
    Telemetry data;
    data.timestamp_ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch())
            .count());

    char hostname[MAX_COMPUTERNAME_LENGTH + 1]{};
    DWORD hostnameSize = sizeof(hostname);
    if (GetComputerNameA(hostname, &hostnameSize)) {
        data.hostname.assign(hostname, hostnameSize);
    }

    for (auto& collector : collectors_) {
        collector->sample(data);
    }

    return data;
}

}
