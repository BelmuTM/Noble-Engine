#pragma once

#include <functional>
#include <string>
#include <variant>

struct ScopeGuard {
    std::function<void()> func;
    bool active = true;

    ~ScopeGuard() { if (active) func(); }
    void release() { active = false; }
};

struct Error {
    std::string              message;
    std::vector<std::string> callstack;

    Error& propagate(const char* function) {
        callstack.emplace_back(function);
        return *this;
    }
};

#define FAIL(msg) Error{(msg), {std::string(__FUNCTION__) + " (" + __FILE__ + ":" + std::to_string(__LINE__) + ")"}}

template <typename T>
class Expected {
public:
    explicit Expected(const T& value) : _data(value) {}
    explicit Expected(T&& value) noexcept : _data(std::move(value)) {}

    explicit Expected(const Error& error) : _data(error) {}
    explicit Expected(Error&& error) noexcept : _data(std::move(error)) {}

    Expected propagate(const char* function) && {
        if (!hasValue()) {
            error().propagate(function);
        }
        return std::move(*this);
    }

    [[nodiscard]] bool hasValue() const noexcept {
        return std::holds_alternative<T>(_data);
    }

    explicit operator bool() const noexcept {
        return hasValue();
    }

    [[nodiscard]]       T& value()       noexcept { return std::get<T>(_data); }
    [[nodiscard]] const T& value() const noexcept { return std::get<T>(_data); }

    [[nodiscard]]       Error& error()       noexcept { return std::get<Error>(_data); }
    [[nodiscard]] const Error& error() const noexcept { return std::get<Error>(_data); }

private:
    std::variant<T, Error> _data;
};

template<>
class Expected<void> {
public:
    Expected() : _hasValue(true) {}

    explicit Expected(const Error& error) : _hasValue(false), _error(error) {}
    Expected(Error&& error) : _hasValue(false), _error(std::move(error)) {}

    [[nodiscard]] bool hasValue() const noexcept {
        return _hasValue;
    }

    explicit operator bool() const noexcept {
        return _hasValue;
    }

    [[nodiscard]]       Error& error()       { return _error; }
    [[nodiscard]] const Error& error() const { return _error; }

private:
    bool _hasValue;
    Error _error;
};

#define TRY_deprecated(expr) \
    do { \
        if (!(expr)) return false; \
    } while (0)

#define TRY_EXPECT(expr)                                       \
    do {                                                       \
        auto _result = (expr);                                 \
        if (!_result) {                                      \
            return std::move(_result).propagate(__FUNCTION__); \
        }                                                      \
    } while (0)
