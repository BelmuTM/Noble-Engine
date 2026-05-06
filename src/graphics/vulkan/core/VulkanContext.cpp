#include "VulkanContext.h"

Expected<void> VulkanContext::create(const Window& window) {
    ScopeGuard guard{[this] { destroy(); }};

    TRY(createVulkanEntity(&capabilities));
    TRY(createVulkanEntity(&instance, capabilities));
    TRY(createVulkanEntity(&surface, instance.handle(), window));
    TRY(createVulkanEntity(&device, capabilities, instance.handle(), surface.handle()));
    TRY(createVulkanEntity(&swapchain, window, device, surface.handle()));

    guard.release();

    return {};
}

void VulkanContext::destroy() {
    flushDeletionQueue();
}
