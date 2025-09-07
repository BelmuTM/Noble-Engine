#include "VulkanContext.h"

#include <cstring>
#include <memory>

#include "core/Engine.h"
#include "core/debug/Logger.h"

VulkanContext::VulkanContext() {
}

VulkanContext::~VulkanContext() {
    shutdown();
}

bool VulkanContext::init() {
    createInstance();
    return true;
}

void VulkanContext::shutdown() {
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

void VulkanContext::drawFrame() {
}

void VulkanContext::createInstance() {
    constexpr VkApplicationInfo applicationInfo{
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "FooBar",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "BarFoo",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VK_API_VERSION_1_4
    };

    auto enabledExtensions = Platform::getVulkanExtensions();

    // Enumerating available extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

    // Ensuring enabled extensions are supported by the drivers
    for (const char* extensionName : enabledExtensions) {
        bool found = false;
        for (VkExtensionProperties available : availableExtensions) {
            if (std::strcmp(extensionName, available.extensionName) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            Engine::fatalExit("Required Vulkan extension not supported: " + std::string(extensionName));
        }
    }

    // Create the instance
    const VkInstanceCreateInfo createInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &applicationInfo,
        .enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size()),
        .ppEnabledExtensionNames = enabledExtensions.data()
    };

    const VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        Engine::fatalExit("Failed to create Vulkan instance");
    }
}
