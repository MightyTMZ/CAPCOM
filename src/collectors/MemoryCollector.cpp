#include "capcom/collectors/MemoryCollector.h"

#include <Windows.h>

namespace capcom {

void MemoryCollector::initialize() {}

void MemoryCollector::sample(Telemetry& data) {
    MEMORYSTATUSEX status{};
    status.dwLength = sizeof(status);

    if (GlobalMemoryStatusEx(&status)) {
        data.memory_percent = static_cast<double>(status.dwMemoryLoad);
    }
}

}
