#include "VulkanInstance.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "core/debug/Logger.h"
#include "core/platform/Platform.h"

#include <unordered_set>
#include <vector>

static const std::vector<const char*> instanceExtensions = {};

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

static const std::vector validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#endif

bool VulkanInstance::create(const VulkanCapabilities& capabilities, std::string& errorMessage) noexcept {
    _capabilities = &capabilities;

    if (!createInstance(errorMessage)) return false;

    _dldi = vk::detail::DispatchLoaderDynamic(_instance, vkGetInstanceProcAddr);

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    if (!setupDebugMessenger(errorMessage)) return false;

#endif

    return true;
}

void VulkanInstance::destroy() noexcept {
#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    if (_debugMessenger) {
        _instance.destroyDebugUtilsMessengerEXT(_debugMessenger, nullptr, _dldi);
        _debugMessenger = VK_NULL_HANDLE;
    }

#endif

    if (_instance) {
        _instance.destroy();
        _instance = VK_NULL_HANDLE;
    }

    _capabilities = nullptr;
}

std::vector<const char*> VulkanInstance::getRequiredExtensions() {
    auto extensions = Platform::getRequiredExtensions();

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#endif

    for (const auto& instanceExtension : instanceExtensions) {
        extensions.push_back(instanceExtension);
    }

    return extensions;
}

bool VulkanInstance::createInstance(std::string& errorMessage) {
    constexpr VkApplicationInfo applicationInfo{
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName   = "Noble Engine",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName        = "NobleEngine",
        .engineVersion      = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion         = VULKAN_VERSION
    };

    // Check if profile is supported
    vk::Bool32 profileSupported = vk::False;
    VK_TRY(vpGetInstanceProfileSupport(_capabilities->handle(), nullptr, &vulkanProfile, &profileSupported), errorMessage);

    if (!profileSupported) {
        errorMessage = "Failed to create Vulkan instance: Vulkan profile not supported.";
        return false;
    }

    // Fetch required extensions and check if they are supported
    const auto extensions = getRequiredExtensions();

    if (extensions.empty()) {
        errorMessage = "Failed to fetch required Vulkan extensions: Vulkan not supported on device.";
        return false;
    }

    auto availableExtensions = VK_CALL(vk::enumerateInstanceExtensionProperties(), errorMessage);
    if (availableExtensions.result != vk::Result::eSuccess) return false;

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

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    layers = validationLayers;

    const auto availableLayers = VK_CALL(vk::enumerateInstanceLayerProperties(), errorMessage);
    if (availableLayers.result != vk::Result::eSuccess) return false;

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
    VkInstanceCreateInfo instanceInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = static_cast<uint32_t>(layers.size()),
        .ppEnabledLayerNames     = layers.data(),
        .enabledExtensionCount   = static_cast<uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    const VpInstanceCreateInfo vpCreateInfo{
        .pCreateInfo             = &instanceInfo,
        .enabledFullProfileCount = 1,
        .pEnabledFullProfiles    = &vulkanProfile
    };

    VkInstance rawInstance{};
    VK_TRY(vpCreateInstance(_capabilities->handle(), &vpCreateInfo, nullptr, &rawInstance), errorMessage);

    _instance = vk::Instance(rawInstance);

    return true;
}

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

vk::Bool32 VulkanInstance::debugCallback(
    const vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
    vk::DebugUtilsMessageTypeFlagsEXT              type,
    const vk::DebugUtilsMessengerCallbackDataEXT*  pCallbackData,
    void*
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
    constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

    constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance);

    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerInfo{};
    debugUtilsMessengerInfo
        .setMessageSeverity(severityFlags)
        .setMessageType(messageTypeFlags)
        .setPfnUserCallback(&debugCallback)
        .setPUserData(this);

    VK_CREATE(_instance.createDebugUtilsMessengerEXT(debugUtilsMessengerInfo, nullptr, _dldi), _debugMessenger,
              errorMessage);

    return true;
}

#endif
