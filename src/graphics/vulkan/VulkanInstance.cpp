#include "VulkanInstance.h"
#include "core/Engine.h"
#include "core/debug/Logger.h"
#include "core/platform/Platform.h"

#include <unordered_set>
#include <vector>

#ifdef VULKAN_DEBUG_UTILS
static const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};
#endif

VulkanInstance::~VulkanInstance() {
    destroy();
}

bool VulkanInstance::create(std::string& errorMessage) noexcept {
    if (!createInstance(errorMessage)) return false;
    setupDebugMessenger();
    return true;
}

void VulkanInstance::destroy() noexcept {
    if (debugMessenger != VK_NULL_HANDLE) {
        destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        debugMessenger = VK_NULL_HANDLE;
    }

    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        instance = VK_NULL_HANDLE;
    }
}

std::vector<const char*> VulkanInstance::getRequiredExtensions() {
    auto extensions = Platform::getVulkanExtensions();

#ifdef VULKAN_DEBUG_UTILS
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    return extensions;
}

bool VulkanInstance::createInstance(std::string& errorMessage) {
    const VkApplicationInfo applicationInfo{
        .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "Bazar",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "BazarEngine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = Engine::VULKAN_VERSION
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
            errorMessage = "Required Vulkan extension not supported: " + std::string(requiredExtension);
            return false;
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
            errorMessage = "Required Vulkan layer not supported: " + std::string(requiredLayer);
            return false;
        }
    }

#endif

    // Create the instance
    const VkInstanceCreateInfo instanceCreateInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames     = layers.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data(),
    };

    const VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        errorMessage = "Failed to create Vulkan instance";
        return false;
    }
    return true;
}

VkBool32 VulkanInstance::debugCallback(const VkDebugUtilsMessageSeverityFlagBitsEXT severity,
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

VkResult VulkanInstance::createDebugUtilsMessengerEXT(const VkInstance                          instance,
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

void VulkanInstance::destroyDebugUtilsMessengerEXT(const VkInstance               instance,
                                                   const VkDebugUtilsMessengerEXT messenger,
                                                   const VkAllocationCallbacks*   pAllocator) {
    const auto destroyDebugUtilsMessengerEXTFunction = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

    if (destroyDebugUtilsMessengerEXTFunction != nullptr) {
        return destroyDebugUtilsMessengerEXTFunction(instance, messenger, pAllocator);
    }
}

void VulkanInstance::setupDebugMessenger() {

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
