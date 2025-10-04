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

#endif //NOBLEENGINE_ERRORHANDLING_H
