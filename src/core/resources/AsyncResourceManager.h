#pragma once

#include "core/debug/ErrorHandling.h"

#include <functional>
#include <future>
#include <memory>
#include <shared_mutex>
#include <type_traits>
#include <unordered_map>

template<typename ResourceType>
class AsyncResourceManager {
public:
    using ResourcePointer = std::unique_ptr<ResourceType>;

    struct ResourceHandle {
        enum class Status : std::uint8_t { Pending, Loading, Ready, Failed };

        std::atomic<Status> status{Status::Pending};
        ResourcePointer     resource;
        Failure             failure;

        [[nodiscard]] bool isReady()  const noexcept {
            return status.load(std::memory_order_acquire) == Status::Ready;
        }

        [[nodiscard]] bool isFailed() const noexcept {
            return status.load(std::memory_order_acquire) == Status::Failed;
        }

        [[nodiscard]] bool isPending() const noexcept {
            auto s = status.load(std::memory_order_acquire);
            return s == Status::Pending || s == Status::Loading;
        }
    };

    using ResourceHandlePointer = std::shared_ptr<ResourceHandle>;

    // Non-const - returns nullptr if not ready yet
    ResourceType* get(const std::string& path) {
        std::shared_lock lock(_mutex);

        auto it = _cache.find(path);
        if (it == _cache.end()) return nullptr;

        if (auto handle = it->second) {
            if (handle->isReady())
                return handle->resource.get();
        }

        return nullptr;
    }

    // Const - returns nullptr if not ready yet
    const ResourceType* get(const std::string& path) const {
        std::shared_lock lock(_mutex);

        auto it = _cache.find(path);
        if (it == _cache.end()) return nullptr;

        if (auto handle = it->second) {
            if (handle->isReady())
                return handle->resource.get();
        }

        return nullptr;
    }

    [[nodiscard]] std::unordered_map<std::string, std::weak_ptr<ResourceHandle>>& getCache() noexcept {
        return _cache;
    }

protected:
    template<typename LoadFunction>
    ResourceHandlePointer loadAsync(const std::string& path, LoadFunction&& loadFunction) {
        static_assert(
            std::is_invocable_r_v<Expected<ResourcePointer>, LoadFunction>,
            "loadFunction must be callable and return Expected<std::unique_ptr<ResourceType>>"
        );

        if (path.empty()) return nullptr;

        // Fast path: resource already in cache
        {
            std::shared_lock readLock(_mutex);
            if (auto it = _cache.find(path); it != _cache.end()) {
                return it->second;
            }
        }

        // Slow path: insert a pending resource handle before releasing the write lock
        std::unique_lock writeLock(_mutex);
        // Another thread won the race
        if (auto it = _cache.find(path); it != _cache.end()) {
            return it->second;
        }

        // Cache a placeholder while the resource is loading
        auto handle = std::make_shared<ResourceHandle>();
        handle->status.store(ResourceHandle::Status::Loading, std::memory_order_relaxed);

        _cache[path] = handle;

        writeLock.unlock();

        // Loading the resource
        Expected<ResourcePointer> result = loadFunction();

        if (result) {
            handle->resource = std::move(result.value());
            handle->status.store(ResourceHandle::Status::Ready, std::memory_order_release);

        } else {
            handle->failure = std::move(result.failure());
            handle->status.store(ResourceHandle::Status::Failed, std::memory_order_release);

            // Cleanup cache placeholder to allow for retries
            std::unique_lock cleanupLock(_mutex);
            _cache.erase(path);
        }

        return handle;
    }

    void cleanupCache(std::function<void(ResourceType&)> destructor) {
        std::unique_lock lock(_mutex);

        // Call the destructor of all resources stored in the cache
        if (destructor) {
            for (auto& [path, handle] : _cache) {
                if (handle) {
                    if (handle->isReady() && handle->resource)
                        destructor(*handle->resource);
                }
            }
        }

        _cache.clear();
    }

private:
    mutable std::shared_mutex _mutex{};

    std::unordered_map<std::string, ResourceHandlePointer> _cache;
};
