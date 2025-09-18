#include "VulkanContext.h"

#include <ranges>

VulkanContext::~VulkanContext() {
    destroy();
}

bool VulkanContext::create(const Platform::Window& window, std::string& errorMessage) {
    const auto rollback = [&](void*) { destroy(); };
    std::unique_ptr<void, decltype(rollback)> guard(nullptr, rollback);

    if (!createVulkanEntity(instance, errorMessage)) return false;
    if (!createVulkanEntity(surface, errorMessage, &instance.getVkInstance(), window)) return false;
    if (!createVulkanEntity(device, errorMessage, instance, surface)) return false;
    if (!createVulkanEntity(swapchain, errorMessage, window, device, surface)) return false;

    guard.release();
    return true;
}

void VulkanContext::destroy() {
    for (auto& destroyFunction : std::ranges::reverse_view(entityDeletionQueue)) {
        if (destroyFunction) destroyFunction();
    }
    entityDeletionQueue.clear();
}
