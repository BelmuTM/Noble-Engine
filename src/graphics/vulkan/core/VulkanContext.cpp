#include "VulkanContext.h"

VulkanContext::~VulkanContext() {
    destroy();
}

bool VulkanContext::create(const Platform::Window& window, std::string& errorMessage) {
    const auto rollback = [&](void*) { destroy(); };
    std::unique_ptr<void, decltype(rollback)> guard(nullptr, rollback);

    if (!createVulkanEntity(&instance, errorMessage)) return false;
    if (!createVulkanEntity(&surface, errorMessage, instance, window)) return false;
    if (!createVulkanEntity(&device, errorMessage, instance, surface)) return false;
    if (!createVulkanEntity(&swapchain, errorMessage, window, device, surface)) return false;

    guard.release();
    return true;
}

void VulkanContext::destroy() {
}
