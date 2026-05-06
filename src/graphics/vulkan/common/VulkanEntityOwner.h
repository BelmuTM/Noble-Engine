#pragma once

#include "VulkanDebugger.h"

#include <functional>
#include <ranges>

template<typename>
class VulkanEntityOwner {
public:
    template<typename Resource, typename... Args>
    Expected<void> createVulkanEntity(Resource* resource, Args&&... args) {

        TRY(resource->create(std::forward<Args>(args)...));

        entityDeletionQueue.push_back([resource] {
            if (!resource) return;
            resource->destroy();
        });

        return {};
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
