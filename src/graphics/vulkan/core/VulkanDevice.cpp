#include "VulkanDevice.h"
#include "graphics/vulkan/common/VulkanDebugger.h"

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

bool VulkanDevice::create(
    const vk::Instance instance, const vk::SurfaceKHR surface, std::string& errorMessage
) noexcept {
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
    if (logicalDevice) {
        logicalDevice.destroy();
        logicalDevice = nullptr;
    }
    _queueFamilyIndices = {};
}

bool VulkanDevice::isPhysicalDeviceSuitable(const vk::PhysicalDevice device) {
    const vk::PhysicalDeviceProperties properties = device.getProperties();
    const vk::PhysicalDeviceFeatures   features   = device.getFeatures();

    // Eliminate current candidate device if critical conditions aren't met
    if (properties.apiVersion < VK_API_VERSION_1_3) {
        return false;
    }

    const auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
    if (availableDeviceExtensions.result != vk::Result::eSuccess) return false;

    std::unordered_set<std::string> availableExtensionNames;
    std::ranges::transform(availableDeviceExtensions.value,
                           std::inserter(availableExtensionNames, availableExtensionNames.end()),
                           [](const vk::ExtensionProperties& ext) { return std::string(ext.extensionName); });

    return std::ranges::all_of(deviceExtensions, [&](const char* requiredExtension) {
        return availableExtensionNames.contains(requiredExtension);
    });
}

bool VulkanDevice::pickPhysicalDevice(const vk::Instance instance, std::string& errorMessage) {
    // Enumerate available devides
    const auto availableDevices = VK_CHECK_RESULT(instance.enumeratePhysicalDevices(), errorMessage);
    if (availableDevices.result != vk::Result::eSuccess) return false;

    // Picking the best suitable candidate within available devices
    auto suitablePhysicalDevices = availableDevices.value | std::views::filter([&](const auto& device) {
        return isPhysicalDeviceSuitable(device);
    });

    const auto it = std::ranges::begin(suitablePhysicalDevices);

    if (it == std::ranges::end(suitablePhysicalDevices)) {
        errorMessage = "Failed to find suitable graphics devices";
        return false;
    }

    physicalDevice = *it;

    Logger::info("Using graphics device \"" + std::string(physicalDevice.getProperties().deviceName) + "\"");
    return true;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface
) {
    QueueFamilyIndices indices;
    const auto queueFamilyProperties = device.getQueueFamilyProperties();

    // Find a queue family with graphics capabilities
    for (int i = 0; i < queueFamilyProperties.size(); i++) {
        if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            break;
        }
    }

    // Find a queue with presentation capabilities
    // Check if the graphics queue family also supports present
    vk::Bool32 presentSupport = device.getSurfaceSupportKHR(indices.graphicsFamily, surface).value;
    // If not found, find a queue family that supports present
    if (!presentSupport) {
        // Find another queue family that support both graphics and present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
            presentSupport = device.getSurfaceSupportKHR(i, surface).value;

            if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics && presentSupport) {
                indices.graphicsFamily = i;
                indices.presentFamily  = i;
                break;
            }
        }
        // If not found, find a queue family that only supports present
        if (indices.presentFamily == UINT32_MAX) {
            for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
                presentSupport = device.getSurfaceSupportKHR(i, surface).value;

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

    vk::DeviceQueueCreateInfo deviceQueueInfo{};
    deviceQueueInfo
        .setQueueFamilyIndex(queueFamilyIndices.graphicsFamily)
        .setQueueCount(1)
        .setQueuePriorities(queuePriority);

    vk::PhysicalDeviceVulkan11Features deviceFeatures_1_1{};
    deviceFeatures_1_1
        .setShaderDrawParameters(vk::True);

    vk::PhysicalDeviceVulkan13Features deviceFeatures_1_3{};
    deviceFeatures_1_3
        .setPNext(&deviceFeatures_1_1)
        .setDynamicRendering(vk::True)
        .setSynchronization2(vk::True);

    vk::DeviceCreateInfo deviceInfo{};
    deviceInfo
        .setPNext(&deviceFeatures_1_3)
        .setQueueCreateInfoCount(1)
        .setQueueCreateInfos(deviceQueueInfo)
        .setPEnabledExtensionNames(deviceExtensions);

    const auto deviceCreate = VK_CHECK_RESULT(physicalDevice.createDevice(deviceInfo), errorMessage);
    if (deviceCreate.result != vk::Result::eSuccess) return false;

    logicalDevice = deviceCreate.value;

    graphicsQueue = logicalDevice.getQueue(queueFamilyIndices.graphicsFamily, 0);
    presentQueue  = logicalDevice.getQueue(queueFamilyIndices.presentFamily , 0);
    return true;
}
