#pragma once

#include <chrono>
#include <functional>
#include <utility>

struct ScopedTimer {
    using Clock    = std::chrono::steady_clock;
    using Duration = std::chrono::duration<double, std::milli>;
    using Callback = std::function<void(std::string, Duration)>;

    explicit ScopedTimer(Callback callback = {})
        : _startTime(Clock::now()), _callback(std::move(callback)) {}

    explicit ScopedTimer(std::string name, Callback callback = {})
        : _name(std::move(name)), _startTime(Clock::now()), _callback(std::move(callback)) {}

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

    ScopedTimer(ScopedTimer&&) = delete;
    ScopedTimer& operator=(ScopedTimer&&) = delete;

    ~ScopedTimer() {
        stop();
    }

    void start(const std::string& name = {}) noexcept {
        _stopped   = false;
        _startTime = Clock::now();

        if (!name.empty()) {
            _name = name;
        }
    }

    void stop() noexcept {
        if (!_stopped) {
            _stopped = true;

            _duration = Clock::now() - _startTime;

            if (_callback) {
                _callback(_name, _duration);
            }
        }
    }

    [[nodiscard]] Duration elapsed() const noexcept {
        return _stopped ? _duration : Clock::now() - _startTime;
    }

private:
    bool _stopped = true;

    std::string _name{};

    Clock::time_point _startTime{};
    Duration          _duration{};

    Callback _callback{};
};
