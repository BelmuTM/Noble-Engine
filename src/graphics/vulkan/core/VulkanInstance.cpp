#include "VulkanInstance.h"
#include "../common/VulkanDebugger.h"

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

bool VulkanInstance::create(std::string& errorMessage) noexcept {
    if (!createInstance(errorMessage))      return false;
    if (!setupDebugMessenger(errorMessage)) return false;
    return true;
}

void VulkanInstance::destroy() noexcept {
    if (debugMessenger) {
        const vk::detail::DispatchLoaderDynamic dldi(instance, vkGetInstanceProcAddr);
        instance.destroyDebugUtilsMessengerEXT(debugMessenger, nullptr, dldi);
        debugMessenger = nullptr;
    }

    if (instance) {
        instance.destroy();
        instance = nullptr;
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
    vk::ApplicationInfo applicationInfo{};
    applicationInfo.pApplicationName   = "Noble";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName        = "NobleEngine";
    applicationInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion         = Engine::VULKAN_VERSION;

    const auto extensions = getRequiredExtensions();

    auto availableExtensions = VK_CHECK_RESULT(vk::enumerateInstanceExtensionProperties(), errorMessage);
    if (availableExtensions.result != vk::Result::eSuccess) {
        return false;
    }

    // Ensure enabled extensions are supported by the drivers
    std::unordered_set<std::string> availableExtensionNames;
    for (auto& [extensionName, specVersion] : availableExtensions.value) {
        availableExtensionNames.insert(extensionName);
    }

    for (const char* requiredExtension : extensions) {
        if (!availableExtensionNames.contains(requiredExtension)) {
            errorMessage = "Required Vulkan extension not supported: " + std::string(requiredExtension);
            return false;
        }
    }

    std::vector<const char*> layers{};

#ifdef VULKAN_DEBUG_UTILS

    layers = validationLayers;

    const auto availableLayers = VK_CHECK_RESULT(vk::enumerateInstanceLayerProperties(), errorMessage);

    if (availableLayers.result != vk::Result::eSuccess) {
        return false;
    }

    // Ensure enabled validation layers are supported by the drivers
    std::unordered_set<std::string> availableLayerNames;
    for (const auto& layer : availableLayers.value) {
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
    vk::InstanceCreateInfo instanceInfo{};
    instanceInfo
        .setPApplicationInfo(&applicationInfo)
        .setPEnabledExtensionNames(extensions)
        .setPEnabledLayerNames(layers);

    const auto instanceCreate = VK_CHECK_RESULT(vk::createInstance(instanceInfo, nullptr), errorMessage);
    if (instanceCreate.result != vk::Result::eSuccess) {
        return false;
    }
    instance = instanceCreate.value;
    return true;
}

vk::Bool32 VulkanInstance::debugCallback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT              type,
    const vk::DebugUtilsMessengerCallbackDataEXT*  pCallbackData, void*
) {
    switch (severity) {
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            Logger::verbose(pCallbackData->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            Logger::info(pCallbackData->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            Logger::warning(pCallbackData->pMessage);
            break;
        case vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            Logger::error(pCallbackData->pMessage);
            break;
        default: break;
    }
    return vk::False;
}

bool VulkanInstance::setupDebugMessenger(std::string& errorMessage) {
#ifdef VULKAN_DEBUG_UTILS

    constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerInfo{};
    debugUtilsMessengerInfo.messageSeverity = severityFlags;
    debugUtilsMessengerInfo.messageType     = messageTypeFlags;
    debugUtilsMessengerInfo.pfnUserCallback = &debugCallback;

    const vk::detail::DispatchLoaderDynamic dldi(instance, vkGetInstanceProcAddr);

    const auto debugUtilsMessengerCreate =
        VK_CHECK_RESULT(instance.createDebugUtilsMessengerEXT(debugUtilsMessengerInfo, nullptr, dldi), errorMessage);

    if (debugUtilsMessengerCreate.result != vk::Result::eSuccess) {
        return false;
    }
    debugMessenger = debugUtilsMessengerCreate.value;

#endif
    return true;
}
