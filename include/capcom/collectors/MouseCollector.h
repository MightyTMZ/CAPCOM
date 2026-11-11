#pragma once
#include "ICollector.h"
#include "InputCounter.h"

namespace capcom {
class MouseCollector : public ICollector {
  public:
    void initialize() override;
    void sample(Telemetry &data) override;

  private:
    InputCounter counter_{false};
};
} // namespace capcom
