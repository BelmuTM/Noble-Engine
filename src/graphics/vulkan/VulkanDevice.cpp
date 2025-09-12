#include "VulkanDevice.h"
#include "VulkanLogger.h"
#include "core/debug/Logger.h"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <vector>

static const std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
};

VulkanDevice::~VulkanDevice() {
    destroy();
}

bool VulkanDevice::create(VkInstance instance, VkSurfaceKHR surface, std::string& errorMessage) noexcept {
    if (!pickPhysicalDevice(instance, errorMessage)) return false;

    _queueFamilyIndices = findQueueFamilies(physicalDevice, surface);

    if (_queueFamilyIndices.graphicsFamily == UINT32_MAX) {
        errorMessage = "Failed to find a queue with graphics capabilities";
        return false;
    }

    if (_queueFamilyIndices.presentFamily == UINT32_MAX) {
        errorMessage = "Failed to find a queue with presentation capabilities";
        return false;
    }

    if (!createLogicalDevice(_queueFamilyIndices, errorMessage)) return false;
    return true;
}

void VulkanDevice::destroy() noexcept {
    if (logicalDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(logicalDevice, nullptr);
        logicalDevice = VK_NULL_HANDLE;
    }
    _queueFamilyIndices = {};
}

bool VulkanDevice::isPhysicalDeviceSuitable(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(device, &properties);
    VkPhysicalDeviceFeatures features;
    vkGetPhysicalDeviceFeatures(device, &features);

    // Eliminate current candidate device if critical conditions aren't met
    if (properties.apiVersion < VK_API_VERSION_1_3) {
        return false;
    }

    // Enumerate available device extensions
    uint32_t deviceExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableDeviceExtensions(deviceExtensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &deviceExtensionCount, availableDeviceExtensions.data());

    std::unordered_set<std::string> availableExtensionNames;
    std::ranges::transform(availableDeviceExtensions,
                           std::inserter(availableExtensionNames, availableExtensionNames.end()),
                           [](const VkExtensionProperties& ext) { return std::string(ext.extensionName); });

    return std::ranges::all_of(deviceExtensions, [&](const char* requiredExtension) {
        return availableExtensionNames.contains(requiredExtension);
    });
}

bool VulkanDevice::pickPhysicalDevice(VkInstance instance, std::string& errorMessage) {
    // Enumerate available devides
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        errorMessage = "Failed to find graphics device with Vulkan support";
        return false;
    }

    std::vector<VkPhysicalDevice> availableDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, availableDevices.data());

    // Picking the best suitable candidate within available devices
    auto suitablePhysicalDevices = availableDevices | std::views::filter([&](const auto& device) {
        return isPhysicalDeviceSuitable(device);
    });

    const auto it = std::ranges::begin(suitablePhysicalDevices);

    if (it == std::ranges::end(suitablePhysicalDevices)) {
        errorMessage = "Failed to find suitable graphics devices";
        return false;
    }

    physicalDevice = *it;

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);

    Logger::info("Using graphics device \"" + std::string(properties.deviceName) + "\"");
    return true;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(
    VkPhysicalDevice device,
    VkSurfaceKHR     surface
) {
    QueueFamilyIndices indices;

    // Enumerate queue family properties
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyPropertyCount, queueFamilyProperties.data());

    // Find a queue family with graphics capabilities
    for (int i = 0; i < queueFamilyProperties.size(); i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
            break;
        }
    }

    // Find a queue with presentation capabilities
    VkBool32 presentSupport = VK_FALSE;
    // Check if the graphics queue family also supports present
    vkGetPhysicalDeviceSurfaceSupportKHR(device, indices.graphicsFamily, surface, &presentSupport);
    // If not found, find a queue family that supports present
    if (!presentSupport) {
        // Find another queue family that support both graphics and present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
                indices.graphicsFamily = i;
                indices.presentFamily  = i;
                break;
            }
        }
        // If not found, find a queue family that only supports present
        if (indices.presentFamily == UINT32_MAX) {
            for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
                if (presentSupport) {
                    indices.presentFamily = i;
                    break;
                }
            }
        }
    } else {
        indices.presentFamily = indices.graphicsFamily;
    }

    return indices;
}

bool VulkanDevice::createLogicalDevice(const QueueFamilyIndices queueFamilyIndices, std::string& errorMessage) {
    // Queue priority is constant and set to one because we are using a single queue
    static constexpr float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyIndices.graphicsFamily,
        .queueCount       = 1,
        .pQueuePriorities = &queuePriority
    };

    VkPhysicalDeviceVulkan13Features deviceFeatures{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
    };

    const VkDeviceCreateInfo deviceCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &deviceFeatures,
        .queueCreateInfoCount    = 1,
        .pQueueCreateInfos       = &deviceQueueCreateInfo,
        .enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data()
    };

    const VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
    if (result != VK_SUCCESS) {
        errorMessage = VK_ERROR_MESSAGE(vkCreateDevice, result);
        return false;
    }

    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, queueFamilyIndices.presentFamily , 0, &presentQueue);
    return true;
}
