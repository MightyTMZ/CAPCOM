#pragma once

#include "ICollector.h"

namespace capcom {

class MemoryCollector final : public ICollector {
public:
    void initialize() override;
    void sample(Telemetry& data) override;
};

}
