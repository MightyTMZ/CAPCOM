#include "capcom/collectors/CPUCollector.h"

#include <Windows.h>

namespace {

uint64_t fileTimeToUint64(const FILETIME& fileTime) {
    ULARGE_INTEGER value{};
    value.LowPart = fileTime.dwLowDateTime;
    value.HighPart = fileTime.dwHighDateTime;
    return value.QuadPart;
}

}

namespace capcom {

void CPUCollector::initialize() {
    FILETIME idle{};
    FILETIME kernel{};
    FILETIME user{};

    if (GetSystemTimes(&idle, &kernel, &user)) {
        previous_idle_ = fileTimeToUint64(idle);
        previous_kernel_ = fileTimeToUint64(kernel);
        previous_user_ = fileTimeToUint64(user);
        initialized_ = true;
    }
}

void CPUCollector::sample(Telemetry& data) {
    FILETIME idle{};
    FILETIME kernel{};
    FILETIME user{};
    if (!GetSystemTimes(&idle, &kernel, &user)) {
        return;
    }

    const uint64_t currentIdle = fileTimeToUint64(idle);
    const uint64_t currentKernel = fileTimeToUint64(kernel);
    const uint64_t currentUser = fileTimeToUint64(user);

    if (initialized_) {
        const uint64_t idleDelta = currentIdle - previous_idle_;
        const uint64_t totalDelta =
            (currentKernel - previous_kernel_) +
            (currentUser - previous_user_);

        if (totalDelta != 0) {
            data.cpu_percent =
                100.0 * static_cast<double>(totalDelta - idleDelta) /
                static_cast<double>(totalDelta);
        }
    }

    previous_idle_ = currentIdle;
    previous_kernel_ = currentKernel;
    previous_user_ = currentUser;
    initialized_ = true;
}

}
