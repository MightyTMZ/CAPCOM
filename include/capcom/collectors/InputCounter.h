#pragma once

#include <Windows.h>
#include <array>
#include <chrono>
#include <deque>
#include <future>
#include <mutex>
#include <thread>

namespace capcom {

// Owns a hook thread. Only event timestamps and transient key-down state are kept.
class InputCounter {
  public:
    explicit InputCounter(bool keyboard) : keyboard_(keyboard) {}
    ~InputCounter();
    void initialize();
    uint32_t count();

  private:
    using Clock = std::chrono::steady_clock;
    void prune(Clock::time_point now);
    static LRESULT CALLBACK callback(int code, WPARAM message, LPARAM event);
    static thread_local InputCounter *current_;
    bool keyboard_;
    DWORD thread_id_ = 0;
    std::thread thread_;
    std::mutex mutex_;
    std::deque<Clock::time_point> events_;
    std::array<bool, 256> down_{};
};
} // namespace capcom
