#include "capcom/collectors/InputCounter.h"
#include <stdexcept>
#include <string>

namespace capcom {
thread_local InputCounter *InputCounter::current_ = nullptr;

InputCounter::~InputCounter() {
    if (thread_.joinable()) {
        PostThreadMessageW(thread_id_, WM_QUIT, 0, 0);
        thread_.join();
    }
}

void InputCounter::initialize() {
    if (thread_.joinable())
        return;
    std::promise<DWORD> ready;
    auto result = ready.get_future();
    thread_ = std::thread([this, ready = std::move(ready)]() mutable {
        current_ = this;
        thread_id_ = GetCurrentThreadId();
        MSG message{};
        PeekMessageW(&message, nullptr, 0, 0, PM_NOREMOVE);
        if (keyboard_) {
            for (int key = 0; key < 256; ++key)
                down_[key] = (GetAsyncKeyState(key) & 0x8000) != 0;
        }
        const HHOOK hook = SetWindowsHookExW(keyboard_ ? WH_KEYBOARD_LL : WH_MOUSE_LL, callback,
                                             GetModuleHandleW(nullptr), 0);
        ready.set_value(hook ? ERROR_SUCCESS : GetLastError());
        if (hook) {
            while (GetMessageW(&message, nullptr, 0, 0) > 0) {
                TranslateMessage(&message);
                DispatchMessageW(&message);
            }
            UnhookWindowsHookEx(hook);
        }
        current_ = nullptr;
    });
    const DWORD error = result.get();
    if (error != ERROR_SUCCESS) {
        thread_.join();
        throw std::runtime_error("Could not install input hook: " + std::to_string(error));
    }
}

void InputCounter::prune(Clock::time_point now) {
    while (!events_.empty() && now - events_.front() >= std::chrono::seconds(2))
        events_.pop_front();
}

uint32_t InputCounter::count() {
    std::lock_guard<std::mutex> lock(mutex_);
    prune(Clock::now());
    return static_cast<uint32_t>(events_.size());
}

LRESULT CALLBACK InputCounter::callback(int code, WPARAM message, LPARAM event) {
    auto *self = current_;
    if (code == HC_ACTION && self) {
        bool pressed = false;
        if (self->keyboard_) {
            const auto &key = *reinterpret_cast<const KBDLLHOOKSTRUCT *>(event);
            if (key.vkCode < self->down_.size()) {
                if (message == WM_KEYDOWN || message == WM_SYSKEYDOWN) {
                    pressed = !self->down_[key.vkCode];
                    self->down_[key.vkCode] = true;
                } else if (message == WM_KEYUP || message == WM_SYSKEYUP) {
                    self->down_[key.vkCode] = false;
                }
            }
        } else {
            pressed =
                message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN;
        }
        if (pressed) {
            std::lock_guard<std::mutex> lock(self->mutex_);
            const auto now = Clock::now();
            self->prune(now);
            self->events_.push_back(now);
        }
    }
    return CallNextHookEx(nullptr, code, message, event);
}
} // namespace capcom
