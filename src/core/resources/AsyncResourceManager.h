#pragma once
#ifndef NOBLEENGINE_ASYNCRESOURCEMANAGER_H
#define NOBLEENGINE_ASYNCRESOURCEMANAGER_H

#include <functional>
#include <future>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>

template<typename ResourceType, template<typename> typename PointerType = std::unique_ptr>
class AsyncResourceManager {
public:
    // Non-const version - returns non-const pointer
    ResourceType* get(const std::string& path) {
        auto it = _cache.find(path);
        if (it == _cache.end()) {
            return nullptr;
        }

        auto& future = it->second;

        if (!future.valid()) {
            return nullptr;
        }

        auto& ptr = future.get();
        return ptr ? ptr.get() : nullptr;
    }

    // Const version - returns const pointer
    const ResourceType* get(const std::string& path) const {
        auto it = _cache.find(path);
        if (it == _cache.end()) {
            return nullptr;
        }

        const auto& future = it->second;

        if (!future.valid()) {
            return nullptr;
        }

        const auto& ptr = future.get();
        return ptr ? ptr.get() : nullptr;
    }

    [[nodiscard]] std::unordered_map<std::string, std::shared_future<PointerType<ResourceType>>>& getCache() noexcept {
        return _cache;
    }

protected:
    static_assert(
        std::is_same_v<PointerType<ResourceType>, std::unique_ptr<ResourceType>> ||
        std::is_same_v<PointerType<ResourceType>, std::shared_ptr<ResourceType>>,
        "PointerType must be std::unique_ptr or std::shared_ptr"
    );

    template<typename LoadFunction>
    std::shared_future<PointerType<ResourceType>> loadAsyncFuture(const std::string& path, LoadFunction&& loadFunc) {
        if (path.empty()) return {};

        using LoadReturnType = decltype(loadFunc());
        static_assert(
            std::is_same_v<LoadReturnType, PointerType<ResourceType>>,
            "loadFunc must return the same pointer type as AsyncResourceManager<...>::PointerType"
        );

        // If resource is already cached, return it
        {
            std::shared_lock readLock(_mutex);
            if (auto it = _cache.find(path); it != _cache.end())
                return it->second;
        }

        std::unique_lock writeLock(_mutex);

        if (auto it = _cache.find(path); it != _cache.end()) {
            return it->second;
        }

        // Mark resource as “loading” using a std::shared_future
        const auto promise = std::make_shared<std::promise<PointerType<ResourceType>>>();
        const auto future  = promise->get_future().share();
        // Cache a placeholder while the resource is loading
        _cache[path] = future;

        writeLock.unlock();

        try {
            PointerType<ResourceType> loadedResource = loadFunc();

            // Failed load
            if (!loadedResource) {
                // Remove cache placeholder
                std::unique_lock cleanupLock(_mutex);
                _cache.erase(path);
                return {};
            }

            // Notify waiters that the resource has done loading
            promise->set_value(std::move(loadedResource));

        } catch(...) {
            promise->set_exception(std::current_exception());

            std::unique_lock cleanupLock(_mutex);
            _cache.erase(path);
        }

        return future;
    }

    void cleanupCache(std::function<void(ResourceType&)> destructor) {
        std::unique_lock lock(_mutex);

        // Call the destructor of each resource stored in the cache
        if (destructor) {
            for (auto& [path, future] : _cache) {
                if (future.valid()) {
                    auto& resourcePtr = future.get();
                    if (resourcePtr) destructor(*resourcePtr);
                }
            }
        }

        _cache.clear();
    }

private:
    std::shared_mutex _mutex{};

    std::unordered_map<std::string, std::shared_future<PointerType<ResourceType>>> _cache;
};

#endif // NOBLEENGINE_ASYNCRESOURCEMANAGER_H
