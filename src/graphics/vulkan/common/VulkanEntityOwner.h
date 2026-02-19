#pragma once

#include <functional>
#include <ranges>
#include <string>

template <typename>
class VulkanEntityOwner {
public:
    template <typename Resource, typename... Args>
    bool createVulkanEntity(Resource* res, std::string& errorMessage, Args&&... args) {
        if (!res->create(std::forward<Args>(args)..., errorMessage)) return false;

        entityDeletionQueue.push_back([res] {
            if (!res) return;
            res->destroy();
        });
        return true;
    }

    void flushDeletionQueue() {
        if (entityDeletionQueue.empty()) return;

        for (auto& destroyFunction : std::ranges::reverse_view(entityDeletionQueue)) {
            if (destroyFunction) destroyFunction();
        }

        entityDeletionQueue.clear();
    }

protected:
    std::vector<std::function<void()>> entityDeletionQueue;
};
