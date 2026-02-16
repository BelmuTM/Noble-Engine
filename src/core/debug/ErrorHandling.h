#pragma once

#include <functional>

struct ScopeGuard {
    std::function<void()> func;
    bool active = true;

    ~ScopeGuard() { if (active) func(); }
    void release() { active = false; }
};

#define TRY(expr) \
    do { \
        if (!(expr)) return false; \
    } while (0)
