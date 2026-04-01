#include "VulkanContext.h"

#include "core/debug/ErrorHandling.h"

bool VulkanContext::create(const Window& window, std::string& errorMessage) {
    ScopeGuard guard{[this] { destroy(); }};

    TRY_BOOL(createVulkanEntity(&capabilities, errorMessage));
    TRY_BOOL(createVulkanEntity(&instance, errorMessage, capabilities));
    TRY_BOOL(createVulkanEntity(&surface, errorMessage, instance.handle(), window));
    TRY_BOOL(createVulkanEntity(&device, errorMessage, capabilities, instance.handle(), surface.handle()));
    TRY_BOOL(createVulkanEntity(&swapchain, errorMessage, window, device, surface.handle()));

    guard.release();

    return true;
}

void VulkanContext::destroy() {
    flushDeletionQueue();
}
