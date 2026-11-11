#include "capcom/collectors/KeyboardCollector.h"

namespace capcom {
    void KeyboardCollector::initialize() { 
        counter_.initialize(); 
    }
    void KeyboardCollector::sample(Telemetry &data) {
        data.keyboard_presses_last_2s = counter_.count();
    }
} // namespace capcom
