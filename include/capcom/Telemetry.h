#pragma once

#include <cstdint>
#include <string>

namespace capcom {

struct Telemetry {
    uint64_t timestamp_ms = 0;
    std::string hostname;

    uint32_t keystrokes = 0;
    uint32_t mouse_events = 0;
    uint32_t mouse_clicks_last_2s = 0;
    uint32_t keyboard_presses_last_2s = 0;

    double cpu_percent = 0.0;
    double memory_percent = 0.0;
    double network_rx_bytes_per_sec = 0.0;
    double network_tx_bytes_per_sec = 0.0;
    double disk_bytes_per_sec = 0.0;

    std::string active_application;
    bool ide_active = false;
};

} // namespace capcom
