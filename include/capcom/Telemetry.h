#pragma once

#include <cstdint>
#include <string>

namespace capcom {
    struct Telemetry {
        uint32_t keystrokes = 0;
        uint32_t mouse_events = 0;

        double cpu_percent = 0.0;
        double memory_percent = 0.0;
        double network_bytes_per_sec = 0.0;
        double disk_byte_per_sec = 0.0;

       std::string active_application;
       bool ide_active = false; 
    };
}