#pragma once

#include <array>
#include <cassert>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>

template<typename Func>
struct ScopeGuard {
    std::decay_t<Func> func;

    bool active = true;

    explicit ScopeGuard(Func f) : func(std::move(f)) {}

    ScopeGuard(ScopeGuard&& other) noexcept
        : func(std::move(other.func)), active(other.active) {
        other.active = false;
    }

    ScopeGuard(const ScopeGuard&)            = delete;
    ScopeGuard& operator=(const ScopeGuard&) = delete;

    ~ScopeGuard() noexcept {
        if (active) func();
    }

    void release() { active = false; }
};

struct Error {
    std::string message;
    std::string domain;
};

struct ErrorFrame {
    const char* function;
    const char* file;
    int         line;
};

struct [[nodiscard]] Failure {
    static constexpr std::size_t MAX_FRAMES = 16;

    Error error;

    std::array<ErrorFrame, MAX_FRAMES> frames{};
    uint8_t count = 0;

    Failure() = default;

    Failure(Error err, const ErrorFrame& frame) : error(std::move(err)) {
        frames[count++] = frame;
    }

    Failure(const Failure&)            = default;
    Failure& operator=(const Failure&) = default;
    Failure(Failure&&)                 = default;
    Failure& operator=(Failure&&)      = default;

    Failure push(const ErrorFrame& frame) && {
        if (count < MAX_FRAMES) {
            frames[count++] = frame;
        } else {
            // Shift all frames one unit to the left
            for (std::size_t i = 1; i < MAX_FRAMES; i++)
                frames[i - 1] = frames[i];

            frames[MAX_FRAMES - 1] = frame;
        }
        return std::move(*this);
    }
};

struct Unexpected {
    Failure failure;

    explicit Unexpected(Failure fail) : failure(std::move(fail)) {}
};

#define FAIL(msg, domain) \
    Unexpected(Failure(Error{msg, domain}, {__FUNCTION__, __FILE__, __LINE__}))

template<typename T>
class [[nodiscard]] Expected {
public:
    explicit Expected(const T& value) : _data(value) {}
    explicit Expected(T&& value) noexcept : _data(std::move(value)) {}

    Expected(Unexpected unexpected) : _data(std::move(unexpected.failure)) {}

    [[nodiscard]] bool failed() const noexcept {
        return !std::holds_alternative<T>(_data);
    }

    explicit operator bool() const noexcept {
        return !failed();
    }

    [[nodiscard]] T& value() noexcept {
        assert(!failed());
        return std::get<T>(_data);
    }

    [[nodiscard]] const T& value() const noexcept {
        assert(!failed());
        return std::get<T>(_data);
    }

    [[nodiscard]] Failure& failure() noexcept {
        assert(failed());
        return std::get<Failure>(_data);
    }

    [[nodiscard]] const Failure& failure() const noexcept {
        assert(failed());
        return std::get<Failure>(_data);
    }

private:
    std::variant<T, Failure> _data;
};

template<>
class [[nodiscard]] Expected<void> {
public:
    Expected() = default;

    Expected(Unexpected unexpected) : _failure(std::move(unexpected.failure)) {}

    [[nodiscard]] bool failed() const noexcept {
        return _failure.has_value();
    }

    explicit operator bool() const noexcept {
        return !failed();
    }

    [[nodiscard]] Failure& failure() {
        assert(_failure.has_value());
        return *_failure;
    }

    [[nodiscard]] const Failure& failure() const {
        assert(_failure.has_value());
        return *_failure;
    }

private:
    std::optional<Failure> _failure;
};

#define TRY_BOOL(expr)             \
    do {                           \
        if (!(expr)) return false; \
    } while (0)

#define TRY(expr)                                      \
    do {                                               \
        auto _result = (expr);                         \
        if (!_result) {                                \
            auto fail = std::move(_result.failure());  \
            return Unexpected{                         \
                std::move(fail).push(                  \
                    {__FUNCTION__, __FILE__, __LINE__} \
                )                                      \
            };                                         \
        }                                              \
    } while (0)

#define TRY_ASSIGN(expr)                                   \
    ([&]() -> decltype(auto) {                             \
        auto _tmp_ = (expr);                               \
        if (!_tmp_)                                        \
            return Unexpected{std::move(_tmp_.failure())}; \
        return _tmp_.value();                              \
    }())
