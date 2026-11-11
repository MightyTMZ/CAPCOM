#pragma once

#include <cstdint>

#include "ICollector.h"

namespace capcom {

class CPUCollector final : public ICollector {
public:
    void initialize() override;
    void sample(Telemetry& data) override;

private:
    uint64_t previous_idle_ = 0;
    uint64_t previous_kernel_ = 0;
    uint64_t previous_user_ = 0;
    bool initialized_ = false;
};

}
