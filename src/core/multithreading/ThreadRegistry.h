#pragma once

#include <string>
#include <thread>

namespace ThreadRegistry {
    void registerThread(const std::string& name);

    void unregisterThread();

    const std::string& currentName();

    std::string getName(std::thread::id id);
}

struct ThreadScope {
    explicit ThreadScope(const std::string& name) {
        ThreadRegistry::registerThread(name);
    }

    ~ThreadScope() {
        ThreadRegistry::unregisterThread();
    }
};
