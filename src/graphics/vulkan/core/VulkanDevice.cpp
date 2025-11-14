#include "VulkanDevice.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/Logger.h"
#include "core/debug/ErrorHandling.h"

#include <algorithm>
#include <ranges>
#include <unordered_set>
#include <vector>

static const std::vector deviceExtensions = {
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

bool VulkanDevice::create(
    const VulkanCapabilities& capabilities,
    const vk::Instance&       instance,
    const vk::SurfaceKHR&     surface,
    std::string&              errorMessage
) noexcept {
    _capabilities = &capabilities;
    _instance     = instance;

    TRY(pickPhysicalDevice(errorMessage));

    _queueFamilyIndices = findQueueFamilies(_physicalDevice, surface);

    if (_queueFamilyIndices.graphicsFamily == UINT32_MAX) {
        errorMessage = "Failed to find a queue with graphics capabilities";
        return false;
    }

    if (_queueFamilyIndices.presentFamily == UINT32_MAX) {
        errorMessage = "Failed to find a queue with presentation capabilities";
        return false;
    }

    TRY(createLogicalDevice(_queueFamilyIndices, errorMessage));
    TRY(createAllocator(errorMessage));

    return true;
}

void VulkanDevice::destroy() noexcept {
    if (_allocator) {
        vmaDestroyAllocator(_allocator);
        _allocator = VK_NULL_HANDLE;
    }

    if (_logicalDevice) {
        _logicalDevice.destroy();
        _logicalDevice = VK_NULL_HANDLE;
    }
    _queueFamilyIndices = {};
}

bool VulkanDevice::isPhysicalDeviceSuitable(const vk::PhysicalDevice device) {
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

bool VulkanDevice::pickPhysicalDevice(std::string& errorMessage) {
    // Enumerate available devides
    const auto availableDevices = VK_CALL(_instance.enumeratePhysicalDevices(), errorMessage);
    if (availableDevices.result != vk::Result::eSuccess) return false;

    // Picking the best suitable candidate within available devices
    auto suitablePhysicalDevices = availableDevices.value | std::views::filter([&](const auto& device) {
        vk::Bool32 profileSupported = vk::False;

        VK_TRY(vpGetPhysicalDeviceProfileSupport(*_capabilities, _instance, device, &vulkanProfile, &profileSupported),
            errorMessage);

        return static_cast<bool>(profileSupported) && isPhysicalDeviceSuitable(device);
    });

    const auto it = std::ranges::begin(suitablePhysicalDevices);

    if (it == std::ranges::end(suitablePhysicalDevices)) {
        errorMessage = "Failed to find suitable graphics devices";
        return false;
    }

    _physicalDevice = *it;

    Logger::info("Using graphics device \"" + std::string(_physicalDevice.getProperties().deviceName) + "\"");

    return true;
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface
) {
    QueueFamilyIndices indices;
    const auto& properties = device.getQueueFamilyProperties2();

    // Find a queue family with graphics capabilities
    for (int i = 0; i < properties.size(); i++) {
        if (properties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
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
        for (size_t i = 0; i < properties.size(); i++) {
            presentSupport = device.getSurfaceSupportKHR(i, surface).value;

            if (properties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics && presentSupport) {
                indices.graphicsFamily = i;
                indices.presentFamily  = i;
                break;
            }
        }
        // If not found, find a queue family that only supports present
        if (indices.presentFamily == UINT32_MAX) {
            for (size_t i = 0; i < properties.size(); i++) {
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

    VkDeviceQueueCreateInfo deviceQueueInfo{};
    deviceQueueInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
    deviceQueueInfo.queueCount       = 1;
    deviceQueueInfo.pQueuePriorities = &queuePriority;

    VkDeviceCreateInfo deviceInfo{};
    deviceInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.queueCreateInfoCount    = 1;
    deviceInfo.pQueueCreateInfos       = &deviceQueueInfo;
    deviceInfo.enabledExtensionCount   = static_cast<uint32_t>(deviceExtensions.size());
    deviceInfo.ppEnabledExtensionNames = deviceExtensions.data();

    VpDeviceCreateInfo vpCreateInfo{};
    vpCreateInfo.pCreateInfo             = &deviceInfo;
    vpCreateInfo.enabledFullProfileCount = 1;
    vpCreateInfo.pEnabledFullProfiles    = &vulkanProfile;

    VkDevice rawDevice = VK_NULL_HANDLE;
    VK_TRY(
        vpCreateDevice(*_capabilities, _physicalDevice, &vpCreateInfo, nullptr, &rawDevice),
        errorMessage
    );

    _logicalDevice = vk::Device(rawDevice);

    _graphicsQueue = _logicalDevice.getQueue(queueFamilyIndices.graphicsFamily, 0);
    _presentQueue  = _logicalDevice.getQueue(queueFamilyIndices.presentFamily , 0);

    return true;
}

bool VulkanDevice::createAllocator(std::string& errorMessage) {
    VmaAllocatorCreateInfo allocatorInfo{};
    allocatorInfo.vulkanApiVersion = VULKAN_VERSION;
    allocatorInfo.physicalDevice   = _physicalDevice;
    allocatorInfo.device           = _logicalDevice;
    allocatorInfo.instance         = _instance;

    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;

    VK_TRY(vmaCreateAllocator(&allocatorInfo, &_allocator), errorMessage);

    return true;
}
