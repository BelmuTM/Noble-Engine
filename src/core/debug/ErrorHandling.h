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

#define TRY(expr) \
    do { \
        if (!(expr)) return false; \
    } while (0)

#endif //NOBLEENGINE_ERRORHANDLING_H
