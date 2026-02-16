#include "VulkanContext.h"

#include "core/debug/ErrorHandling.h"

bool VulkanContext::create(const Window& window, std::string& errorMessage) {
    ScopeGuard guard{[this] { destroy(); }};

    TRY(createVulkanEntity(&capabilities, errorMessage));
    TRY(createVulkanEntity(&instance, errorMessage, capabilities));
    TRY(createVulkanEntity(&surface, errorMessage, instance.handle(), window));
    TRY(createVulkanEntity(&device, errorMessage, capabilities, instance.handle(), surface.handle()));
    TRY(createVulkanEntity(&swapchain, errorMessage, window, device, surface.handle()));

    guard.release();

    return true;
}

void VulkanContext::destroy() {
    flushDeletionQueue();
}
