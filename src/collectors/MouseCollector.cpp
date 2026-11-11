#include "capcom/collectors/MouseCollector.h"

namespace capcom {
    void MouseCollector::initialize() { 
        counter_.initialize(); 
    }
    void MouseCollector::sample(Telemetry &data) { 
        data.mouse_clicks_last_2s = counter_.count(); 
    }
} // namespace capcom
