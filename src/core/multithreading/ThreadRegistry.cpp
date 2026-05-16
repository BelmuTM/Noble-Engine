#include "ThreadRegistry.h"

#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace ThreadRegistry {
    static constexpr auto DEFAULT_THREAD_NAME = "Undefined_Thread";

    namespace {
        thread_local std::string threadNameLocal = DEFAULT_THREAD_NAME;

        std::shared_mutex mutex{};

        std::unordered_map<std::thread::id, std::string> threadRegistry{};
    }

    void registerThread(const std::string& name) {
        threadNameLocal = name; // Thread-local, no lock needed

        std::unique_lock lock(mutex);
        threadRegistry[std::this_thread::get_id()] = name;
    }

    void unregisterThread() {
        threadNameLocal = DEFAULT_THREAD_NAME; // Before lock in case of throws

        std::unique_lock lock(mutex);
        threadRegistry.erase(std::this_thread::get_id());
    }

    const std::string& currentName() { return threadNameLocal; }

    std::string getName(const std::thread::id id) {
        std::shared_lock lock(mutex);
        const auto it = threadRegistry.find(id);
        return it != threadRegistry.end() ? it->second : DEFAULT_THREAD_NAME;
    }
}
