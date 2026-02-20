#include "VulkanContext.h"

#include "core/debug/ErrorHandling.h"

bool VulkanContext::create(const Window& window, std::string& errorMessage) {
    ScopeGuard guard{[this] { destroy(); }};

    TRY_deprecated(createVulkanEntity(&capabilities, errorMessage));
    TRY_deprecated(createVulkanEntity(&instance, errorMessage, capabilities));
    TRY_deprecated(createVulkanEntity(&surface, errorMessage, instance.handle(), window));
    TRY_deprecated(createVulkanEntity(&device, errorMessage, capabilities, instance.handle(), surface.handle()));
    TRY_deprecated(createVulkanEntity(&swapchain, errorMessage, window, device, surface.handle()));

    guard.release();

    return true;
}

void VulkanContext::destroy() {
    flushDeletionQueue();
}
