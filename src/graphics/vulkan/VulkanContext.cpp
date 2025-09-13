#include "VulkanContext.h"
#include "core/Engine.h"

VulkanContext::~VulkanContext() {
    shutdown();
}

bool VulkanContext::init(const Platform::Window& window) {
    std::string errorMessage;

    if (!instance.create(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!surface.create(&instance.getVkInstance(), window, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!device.create(instance, surface, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    if (!swapchain.create(window, device, surface, errorMessage)) {
        Engine::fatalExit(errorMessage);
    }
    return true;
}

void VulkanContext::shutdown() {
    swapchain.destroy();
    surface.destroy();
    device.destroy();
    instance.destroy();
}

void VulkanContext::drawFrame() {
}
