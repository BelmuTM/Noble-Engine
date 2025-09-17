#include "VulkanContext.h"

#include <ranges>

#include "core/debug/Logger.h"

VulkanContext::~VulkanContext() {
    shutdown();
}

bool VulkanContext::init(const Platform::Window& window) {
    std::string errorMessage;
    createVulkanEntity(instance, errorMessage);
    createVulkanEntity(surface, errorMessage, &instance.getVkInstance(), window);
    createVulkanEntity(device, errorMessage, instance, surface);
    createVulkanEntity(swapchain, errorMessage, window, device, surface);
    createVulkanEntity(pipeline, errorMessage, &device.getLogicalDevice(), swapchain);
    Logger::info("Created Vulkan entities");
    return true;
}

void VulkanContext::shutdown() {
    // Iterate each destroy function and call them
    for (auto & it : std::ranges::reverse_view(entityDeletionQueue)) {
        it();
    }
    entityDeletionQueue.clear();
    Logger::info("Destroyed Vulkan entities");
}

void VulkanContext::drawFrame() {
}
