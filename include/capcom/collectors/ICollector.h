#pragma once

#include "../Telemetry.h"

namespace capcom {

    class ICollector {
    public:
        virtual ~ICollector() = default;

        virtual void initialize() = 0;
        virtual void sample(Telemetry& data) = 0;
    };
}