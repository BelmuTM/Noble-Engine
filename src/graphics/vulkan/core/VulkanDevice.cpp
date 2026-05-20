#include "VulkanDevice.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/Logger.h"

#include <algorithm>
#include <ranges>
#include <string>
#include <unordered_set>
#include <vector>

static const std::vector deviceExtensions = {
    VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Expected<void> VulkanDevice::create(
    const VulkanCapabilities& capabilities,
    const vk::Instance&       instance,
    const vk::SurfaceKHR&     surface
) noexcept {
    _capabilities = &capabilities;
    _instance     = instance;

    TRY(pickPhysicalDevice());

    _queueFamilyIndices = findQueueFamilies(_physicalDevice, surface);

    if (_queueFamilyIndices.graphicsFamily == UINT32_MAX) {
        return VK_FAIL("Failed to find a queue with graphics capabilities.");
    }

    if (_queueFamilyIndices.presentFamily == UINT32_MAX) {
        return VK_FAIL("Failed to find a queue with presentation capabilities.");
    }

    TRY(createLogicalDevice(_queueFamilyIndices));
    TRY(createAllocator());
    TRY(createQueryPool());

    return {};
}

void VulkanDevice::destroy() noexcept {
    if (_queryPool && _logicalDevice) {
        _logicalDevice.destroyQueryPool(_queryPool);
        _queryPool = VK_NULL_HANDLE;
    }

    if (_allocator) {
        vmaDestroyAllocator(_allocator);
        _allocator = VK_NULL_HANDLE;
    }

    if (_logicalDevice) {
        _logicalDevice.destroy();
        _logicalDevice = VK_NULL_HANDLE;
    }

    _capabilities = nullptr;

    _queueFamilyIndices = {};
}

bool VulkanDevice::isPhysicalDeviceSuitable(const vk::PhysicalDevice device) {
    const vk::PhysicalDeviceProperties2& deviceProperties = device.getProperties2();

    // Eliminate current candidate device if critical conditions aren't met
    if (deviceProperties.properties.apiVersion < VK_API_VERSION_1_3) {
        return false;
    }

    const auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
    if (availableDeviceExtensions.result != vk::Result::eSuccess) return false;

    std::unordered_set<std::string_view> availableExtensionNames;

    std::ranges::transform(
        availableDeviceExtensions.value,
        std::inserter(availableExtensionNames, availableExtensionNames.end()),
        [](const vk::ExtensionProperties& ext) { 
            return std::string_view(ext.extensionName.data());
        }
    );

    return std::ranges::all_of(deviceExtensions, [&](const char* requiredExtension) {
        return availableExtensionNames.contains(requiredExtension);
    });
}

Expected<void> VulkanDevice::pickPhysicalDevice() {
    const auto availableDevices = VK_EXPECT(_instance.enumeratePhysicalDevices());
    TRY(availableDevices);

    // Pick the best suitable candidate within available devices
    std::vector<vk::PhysicalDevice> discreteCandidates;
    std::vector<vk::PhysicalDevice> integratedCandidates;

    for (const auto& device : availableDevices.value()) {
        vk::Bool32 profileSupported = vk::False;

        VK_TRY(vpGetPhysicalDeviceProfileSupport(
            _capabilities->handle(), _instance, device, &vulkanProfile, &profileSupported
        ));

        if (!static_cast<bool>(profileSupported) || !isPhysicalDeviceSuitable(device)) {
            continue;
        }

        const auto& deviceProperties = device.getProperties2();

        if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu) {
            discreteCandidates.push_back(device);
        } else if (deviceProperties.properties.deviceType == vk::PhysicalDeviceType::eIntegratedGpu) {
            integratedCandidates.push_back(device);
        }
    }

    if (!discreteCandidates.empty()) {
        _physicalDevice = discreteCandidates.front();
    } else if (!integratedCandidates.empty()) {
        _physicalDevice = integratedCandidates.front();
    } else {
        return VK_FAIL("Failed to find suitable graphics devices.");
    }

    Logger::info("Using graphics device \"" + std::string(_physicalDevice.getProperties().deviceName.data()) + "\"");

    return {};
}

VulkanDevice::QueueFamilyIndices VulkanDevice::findQueueFamilies(
    const vk::PhysicalDevice device, const vk::SurfaceKHR surface
) {
    QueueFamilyIndices indices;
    const auto& properties = device.getQueueFamilyProperties2();

    const auto propertiesSize = static_cast<uint32_t>(properties.size());

    // Find a queue family with graphics capabilities
    for (std::uint32_t i = 0; i < propertiesSize; i++) {
        if (properties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics) {
            indices.graphicsFamily = i;
            break;
        }
    }

    // Find a queue with presentation capabilities
    // Check if the graphics queue family also supports present
    vk::Bool32 presentSupport = device.getSurfaceSupportKHR(indices.graphicsFamily, surface).value;

    // If not found, find a dedicated queue family that supports present
    if (!presentSupport) {

        // Find another queue family that support both graphics and present
        for (std::uint32_t i = 0; i < propertiesSize; i++) {
            presentSupport = device.getSurfaceSupportKHR(i, surface).value;

            if (properties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eGraphics && presentSupport) {
                indices.graphicsFamily = i;
                indices.presentFamily  = i;
                break;
            }
        }

        // If not found, find a queue family that only supports present
        if (indices.presentFamily == UINT32_MAX) {
            for (std::uint32_t i = 0; i < propertiesSize; i++) {
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

    // Find a dedicated queue family with compute capabilities
    for (std::uint32_t i = 0; i < propertiesSize; i++) {
        const auto flags = properties[i].queueFamilyProperties.queueFlags;

        if (flags & vk::QueueFlagBits::eCompute && !(flags & vk::QueueFlagBits::eGraphics)) {
            indices.computeFamily = i;
            break;
        }
    }

    // If not found, fallback to any queue with compute capabilities
    if (indices.computeFamily != UINT32_MAX) {
        for (std::uint32_t i = 0; i < propertiesSize; i++) {
            if (properties[i].queueFamilyProperties.queueFlags & vk::QueueFlagBits::eCompute) {
                indices.computeFamily = i;
                break;
            }
        }
    }

    return indices;
}

Expected<void> VulkanDevice::createLogicalDevice(const QueueFamilyIndices queueFamilyIndices) {
    // Queue priority is constant and set to one because we are using a single queue
    static constexpr float queuePriority = 1.0f;

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

    std::unordered_set uniqueQueueFamilies{
        queueFamilyIndices.graphicsFamily,
        queueFamilyIndices.presentFamily
    };

    if (queueFamilyIndices.computeFamily != UINT32_MAX) {
        uniqueQueueFamilies.insert(queueFamilyIndices.computeFamily);
    }

    for (const std::uint32_t& family : uniqueQueueFamilies) {
        queueCreateInfos.push_back(
            VkDeviceQueueCreateInfo{
                .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .queueFamilyIndex = family,
                .queueCount       = 1,
                .pQueuePriorities = &queuePriority
            }
        );
    }

    VkPhysicalDeviceFeatures deviceFeatures{
        .fillModeNonSolid        = vk::True,
        .wideLines               = vk::True,
        .samplerAnisotropy       = vk::True,
        .pipelineStatisticsQuery = vk::True
    };

    VkPhysicalDeviceVulkan11Features deviceFeatures_1_1{
        .shaderDrawParameters = vk::True
    };

    VkPhysicalDeviceVulkan13Features deviceFeatures_1_3{
        .pNext            = &deviceFeatures_1_1,
        .synchronization2 = vk::True,
        .dynamicRendering = vk::True
    };

    VkPhysicalDeviceBufferDeviceAddressFeatures deviceBufferAddressFeatures{
        .pNext               = &deviceFeatures_1_3,
        .bufferDeviceAddress = vk::True
    };

    VkPhysicalDeviceDynamicRenderingUnusedAttachmentsFeaturesEXT deviceDynamicRenderingFeatures{
        .pNext = &deviceBufferAddressFeatures,
    };

    VkDeviceCreateInfo deviceInfo{
        .sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext                   = &deviceDynamicRenderingFeatures,
        .queueCreateInfoCount    = static_cast<std::uint32_t>(queueCreateInfos.size()),
        .pQueueCreateInfos       = queueCreateInfos.data(),
        .enabledExtensionCount   = static_cast<std::uint32_t>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures        = &deviceFeatures
    };

    const VpDeviceCreateInfo vpCreateInfo{
        .pCreateInfo             = &deviceInfo,
        .enabledFullProfileCount = 1,
        .pEnabledFullProfiles    = &vulkanProfile
    };

    VkDevice rawDevice = VK_NULL_HANDLE;
    VK_TRY(vpCreateDevice(_capabilities->handle(), _physicalDevice, &vpCreateInfo, nullptr, &rawDevice));

    _logicalDevice = vk::Device(rawDevice);

    _graphicsQueue = _logicalDevice.getQueue(queueFamilyIndices.graphicsFamily, 0);
    _presentQueue  = _logicalDevice.getQueue(queueFamilyIndices.presentFamily , 0);

    if (queueFamilyIndices.computeFamily != UINT32_MAX) {
        _computeQueue = _logicalDevice.getQueue(queueFamilyIndices.computeFamily, 0);
    }

    return {};
}

Expected<void> VulkanDevice::createAllocator() {
    const VmaAllocatorCreateInfo allocatorInfo{
        .flags            = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice   = _physicalDevice,
        .device           = _logicalDevice,
        .instance         = _instance,
        .vulkanApiVersion = VULKAN_VERSION,
    };

    VK_TRY(vmaCreateAllocator(&allocatorInfo, &_allocator));

    return {};
}

Expected<void> VulkanDevice::createQueryPool() {
    vk::QueryPoolCreateInfo queryPoolInfo{};
    queryPoolInfo
        .setQueryType(vk::QueryType::ePipelineStatistics)
        .setQueryCount(1)
        .setPipelineStatistics(vk::QueryPipelineStatisticFlagBits::eInputAssemblyPrimitives);

    VK_CREATE(_queryPool, _logicalDevice.createQueryPool(queryPoolInfo));

    return {};
}
