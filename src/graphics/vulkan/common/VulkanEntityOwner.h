#pragma once
#ifndef NOBLEENGINE_VULKANENTITYOWNER_H
#define NOBLEENGINE_VULKANENTITYOWNER_H

#include <functional>
#include <ranges>

template<typename Derived>
class VulkanEntityOwner {
protected:
    std::vector<std::function<void()>> entityDeletionQueue;

public:
    template<typename Resource, typename... Args>
    bool createVulkanEntity(Resource* res, std::string& errorMessage, Args&&... args) {
        if (!res->create(std::forward<Args>(args)..., errorMessage)) return false;
        entityDeletionQueue.push_back([res] { res->destroy(); });
        return true;
    }

    ~VulkanEntityOwner() {
        for (auto& destroyFunction : std::ranges::reverse_view(entityDeletionQueue)) {
            if (destroyFunction) destroyFunction();
        }
        entityDeletionQueue.clear();
    }
};

#endif //NOBLEENGINE_VULKANENTITYOWNER_H