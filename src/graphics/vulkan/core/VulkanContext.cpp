#include "VulkanContext.h"
#include "core/debug/ErrorHandling.h"

VulkanContext::~VulkanContext() {
    destroy();
}

bool VulkanContext::create(const Platform::Window& window, std::string& errorMessage) {
    ScopeGuard guard{[this] { destroy(); }};

    TRY(createVulkanEntity(&instance, errorMessage));
    TRY(createVulkanEntity(&surface, errorMessage, instance, window));
    TRY(createVulkanEntity(&device, errorMessage, instance, surface));
    TRY(createVulkanEntity(&swapchain, errorMessage, window, device, surface));

    guard.release();
    return true;
}

void VulkanContext::destroy() {
    flushDeletionQueue();
}
