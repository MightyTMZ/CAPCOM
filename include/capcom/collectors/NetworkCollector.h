#pragma once

#include <chrono>
#include <cstdint>

#include "ICollector.h"

namespace capcom {

class NetworkCollector final : public ICollector {
public:
    void initialize() override;
    void sample(Telemetry& data) override;

private:
    uint64_t previous_rx_ = 0;
    uint64_t previous_tx_ = 0;
    std::chrono::steady_clock::time_point previous_time_{};
    bool initialized_ = false;
};

}
