#include "VulkanContext.h"
#include "core/Engine.h"
#include "core/debug/Logger.h"

#include <algorithm>
#include <memory>
#include <ranges>
#include <unordered_set>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#include <vulkan/vulkan_win32.h>
#endif

constexpr uint32_t VULKAN_VERSION = VK_API_VERSION_1_4;

std::vector deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    VK_KHR_SPIRV_1_4_EXTENSION_NAME,
    VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
    VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME
};

#ifdef VULKAN_DEBUG_UTILS
static const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
#endif

VulkanContext::~VulkanContext() {
    shutdown();
}

bool VulkanContext::init(const WindowHandle& _window) {
    window = _window;
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    return true;
}

void VulkanContext::shutdown() {
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        surface = VK_NULL_HANDLE;
    }

    if (logicalDevice != VK_NULL_HANDLE) {
        vkDestroyDevice(logicalDevice, nullptr);
        logicalDevice = VK_NULL_HANDLE;
    }

    if (debugMessenger != VK_NULL_HANDLE) {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

void VulkanContext::drawFrame() {
}

std::vector<const char*> VulkanContext::getRequiredExtensions() {
    auto extensions = Platform::getVulkanExtensions();
#ifdef VULKAN_DEBUG_UTILS
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    return extensions;
}

void VulkanContext::createInstance() {
    constexpr VkApplicationInfo applicationInfo{
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "Bazar",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "BazarEngine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VULKAN_VERSION
    };

    const auto extensions = getRequiredExtensions();
    // Enumerate available instance extensions
    uint32_t instanceExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(instanceExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtensionCount, availableExtensions.data());

    // Ensure enabled extensions are supported by the drivers
    std::unordered_set<std::string> availableExtensionNames;
    for (auto& [extensionName, specVersion] : availableExtensions) {
        availableExtensionNames.insert(extensionName);
    }

    for (const char* requiredExtension : extensions) {
        if (!availableExtensionNames.contains(requiredExtension)) {
            Engine::fatalExit("Required Vulkan extension not supported: " + std::string(requiredExtension));
        }
    }

    uint32_t                 layerCount = 0;
    std::vector<const char*> layers     = {};

#ifdef VULKAN_DEBUG_UTILS

    layers = validationLayers;
    // Enumerate available validation layers
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    // Ensure enabled validation layers are supported by the drivers
    std::unordered_set<std::string> availableLayerNames;
    for (const auto& layer : availableLayers) {
        availableLayerNames.insert(layer.layerName);
    }

    for (const char* requiredLayer : layers) {
        if (!availableLayerNames.contains(requiredLayer)) {
            Engine::fatalExit("Required Vulkan validation layer not supported: " + std::string(requiredLayer));
        }
    }

#endif

    // Create the instance
    const VkInstanceCreateInfo instanceCreateInfo{
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames     = layers.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    const VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        Engine::fatalExit("Failed to create Vulkan instance");
    }
}

VkBool32 VulkanContext::debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
                                      VkDebugUtilsMessageTypeFlagsEXT              type,
                                      const VkDebugUtilsMessengerCallbackDataEXT*  pCallbackData, void*) {
    switch (severity) {
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
            Logger::verbose(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
            Logger::info(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
            Logger::warning(pCallbackData->pMessage);
            break;
        case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
            Logger::error(pCallbackData->pMessage);
            break;
        default: break;
    }
    return VK_FALSE;
}

VkResult VulkanContext::createDebugUtilsMessengerEXT(const VkInstance                          instance,
                                                     const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                     const VkAllocationCallbacks*              pAllocator,
                                                     VkDebugUtilsMessengerEXT*                 pMessenger) {
    const auto createDebugUtilsMessengerEXTFunction = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

    if (createDebugUtilsMessengerEXTFunction != nullptr) {
        return createDebugUtilsMessengerEXTFunction(instance, pCreateInfo, pAllocator, pMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanContext::destroyDebugUtilsMessengerEXT(const VkInstance               instance,
                                                  const VkDebugUtilsMessengerEXT messenger,
                                                  const VkAllocationCallbacks*   pAllocator) {
    const auto destroyDebugUtilsMessengerEXTFunction = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (destroyDebugUtilsMessengerEXTFunction != nullptr) {
        return destroyDebugUtilsMessengerEXTFunction(instance, messenger, pAllocator);
    }
}

void VulkanContext::setupDebugMessenger() {
#ifdef VULKAN_DEBUG_UTILS

    constexpr VkDebugUtilsMessageSeverityFlagsEXT severityFlags(
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT);

    constexpr VkDebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT);

    constexpr VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfo{
        .sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = severityFlags,
        .messageType     = messageTypeFlags,
        .pfnUserCallback = &debugCallback
    };

    const VkResult result = createDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCreateInfo, nullptr,
                                                         &debugMessenger);

    if (result != VK_SUCCESS) {
        Logger::error("Failed to create Vulkan debug utils messenger");
    }
#endif
}

void VulkanContext::createSurface() {
    if (!window) {
        Engine::fatalExit("Failed to create Vulkan window surface: window handle is null");
    }

#if defined(_WIN32) || defined(_WIN64)

    const VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .hinstance = GetModuleHandle(nullptr),
        .hwnd      = static_cast<HWND>(window)
    };

    const VkResult result = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        Engine::fatalExit("Failed to create Vulkan window surface");
    }

#elif defined(__linux__)
    // TO-DO
#endif
}

bool VulkanContext::isPhysicalDeviceSuitable(const VkPhysicalDevice device) {
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

void VulkanContext::pickPhysicalDevice() {
    // Enumerate available devides
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        Logger::error("Failed to find graphics devices with Vulkan support");
        return;
    }

    std::vector<VkPhysicalDevice> availableDevices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, availableDevices.data());

    // Picking the best suitable candidate within available devices
    auto suitablePhysicalDevices = availableDevices | std::views::filter([&](const auto& device) {
        return isPhysicalDeviceSuitable(device);
    });

    const auto it = std::ranges::begin(suitablePhysicalDevices);

    if (it != std::ranges::end(suitablePhysicalDevices)) {
        physicalDevice = *it;

        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicalDevice, &properties);

        VkSurfaceCapabilitiesKHR surfaceCapabilitiesKHR{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilitiesKHR);

        uint32_t surfaceFormatCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);
        std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats.data());

        uint32_t surfacePresentModeCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, nullptr);
        std::vector<VkPresentModeKHR> surfacePresentModes(surfacePresentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &surfacePresentModeCount, surfacePresentModes.data());

        Logger::info("Using graphics device \"" + std::string(properties.deviceName) + "\"");
    } else {
        Engine::fatalExit("Failed to find suitable graphics devices");
    }
}

void VulkanContext::createLogicalDevice() {
    // Enumerate queue family properties
    uint32_t queueFamilyPropertyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties.data());

    // Find a queue family with graphics capabilities
    uint32_t graphicsFamilyIndex = -1;
    for (int i = 0; i < queueFamilyProperties.size(); i++) {
        if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsFamilyIndex = i;
            break;
        }
    }

    if (graphicsFamilyIndex == -1) {
        Logger::error("Failed to find a queue with graphics capabilities");
        return;
    }

    // Find a queue with presentation capabilities
    uint32_t presentFamilyIndex = -1;
    VkBool32 presentSupport     = VK_FALSE;
    // Check if the graphics queue family also supports present
    vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, graphicsFamilyIndex, surface, &presentSupport);
    // If not found, find a queue family that supports present
    if (!presentSupport) {
        // Find another queue family that support both graphics and present
        for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport) {
                graphicsFamilyIndex = i;
                presentFamilyIndex  = i;
                break;
            }
        }
        // If not found, find a queue family that only supports present
        if (presentFamilyIndex == -1) {
            for (size_t i = 0; i < queueFamilyProperties.size(); i++) {
                vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
                if (presentSupport) {
                    presentFamilyIndex = i;
                    break;
                }
            }
        }
    } else {
        presentFamilyIndex = graphicsFamilyIndex;
    }

    if (presentFamilyIndex == -1) {
        Logger::error("Failed to find a queue with presentation capabilities");
        return;
    }

    // Queue priority is constant and set to one because we are using a single queue
    static constexpr float queuePriority = 0.0f;

    VkDeviceQueueCreateInfo deviceQueueCreateInfo{
        .sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = graphicsFamilyIndex,
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

    if (result == VK_SUCCESS) {
        vkGetDeviceQueue(logicalDevice, graphicsFamilyIndex, 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, presentFamilyIndex , 0, &presentQueue );
    } else {
        Logger::error("Failed to create Vulkan logical device");
    }
}
