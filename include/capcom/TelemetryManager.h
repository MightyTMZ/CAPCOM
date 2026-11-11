
#pragma once

#include <memory>
#include <vector>

#include "Telemetry.h"
#include "collectors/ICollector.h"

namespace capcom {

class TelemetryManager {
public:
    void addCollector(std::unique_ptr<ICollector> collector);
    void initialize();
    Telemetry collect();

private:
    std::vector<std::unique_ptr<ICollector>> collectors_;
};

}
