#pragma once
#ifndef NOBLEENGINE_ERRORHANDLING_H
#define NOBLEENGINE_ERRORHANDLING_H

#include <functional>

struct ScopeGuard {
    std::function<void()> func;
    bool active = true;

    ~ScopeGuard() { if (active) func(); }
    void release() { active = false; }
};

template<typename T>
class Expected {
public:
    explicit Expected(T value) : _hasValue(true), _value(std::move(value)) {}
    explicit Expected(std::string error) : _hasValue(false), _error(std::move(error)) {}

    [[nodiscard]] bool hasValue() const { return _hasValue; }

    explicit operator bool() const { return _hasValue; }

    T value() const { return _value; }

private:
    bool _hasValue;
    T _value{};

    std::string _error;
};

#endif //NOBLEENGINE_ERRORHANDLING_H
