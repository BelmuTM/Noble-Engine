#include "VulkanContext.h"
#include "core/debug/ErrorHandling.h"

VulkanContext::~VulkanContext() {
    destroy();
}

bool VulkanContext::create(const Platform::Window& window, std::string& errorMessage) {
    ScopeGuard guard{[this] { destroy(); }};

    if (!createVulkanEntity(&instance, errorMessage))                           return false;
    if (!createVulkanEntity(&surface, errorMessage, instance, window))          return false;
    if (!createVulkanEntity(&device, errorMessage, instance, surface))          return false;
    if (!createVulkanEntity(&swapchain, errorMessage, window, device, surface)) return false;

    guard.release();
    return true;
}

void VulkanContext::destroy() {
    flushDeletionQueue();
}
