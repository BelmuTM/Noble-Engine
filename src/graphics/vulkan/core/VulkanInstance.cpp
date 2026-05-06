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

Expected<void> VulkanInstance::create(const VulkanCapabilities& capabilities) noexcept {
    _capabilities = &capabilities;

    TRY(createInstance());

    _dispatchLoader = vk::detail::DispatchLoaderDynamic(_instance, vkGetInstanceProcAddr);

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    TRY(setupDebugMessenger());

#endif

    return {};
}

void VulkanInstance::destroy() noexcept {
#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    if (_debugMessenger) {
        _instance.destroyDebugUtilsMessengerEXT(_debugMessenger, nullptr, _dispatchLoader);
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

Expected<void> VulkanInstance::createInstance() {
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
    VK_TRY(vpGetInstanceProfileSupport(_capabilities->handle(), nullptr, &vulkanProfile, &profileSupported));

    if (!profileSupported) {
        return VK_FAIL("Failed to create instance: profile not supported.");
    }

    // Fetch required extensions and check if they are supported
    const auto extensions = getRequiredExtensions();

    if (extensions.empty()) {
        return VK_FAIL("Failed to fetch required extensions: Vulkan not supported on device.");
    }

    const auto availableExtensions = VK_EXPECT(vk::enumerateInstanceExtensionProperties());
    TRY(availableExtensions);

    // Ensure enabled extensions are supported by the drivers
    std::unordered_set<std::string> availableExtensionNames;
    for (auto& [extensionName, specVersion] : availableExtensions.value()) {
        availableExtensionNames.insert(extensionName);
    }

    for (const char* requiredExtension : extensions) {
        if (!availableExtensionNames.contains(requiredExtension)) {
            return VK_FAIL("Required extension not supported: " + std::string(requiredExtension));
        }
    }

    std::vector<const char*> layers{};

#ifdef VULKAN_VALIDATION_LAYERS_ENABLED

    layers = validationLayers;

    Failure enumerateFailure;

    const auto availableLayers = VK_EXPECT(vk::enumerateInstanceLayerProperties());
    TRY(availableLayers);

    // Ensure enabled validation layers are supported by the drivers
    std::unordered_set<std::string> availableLayerNames;
    for (const auto& layer : availableLayers.value()) {
        availableLayerNames.insert(layer.layerName);
    }

    for (const char* requiredLayer : layers) {
        if (!availableLayerNames.contains(requiredLayer)) {
            return VK_FAIL("Required layer not supported: " + std::string(requiredLayer));
        }
    }

#endif

    // Create the instance
    VkInstanceCreateInfo instanceInfo{
        .sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo        = &applicationInfo,
        .enabledLayerCount       = static_cast<std::uint32_t>(layers.size()),
        .ppEnabledLayerNames     = layers.data(),
        .enabledExtensionCount   = static_cast<std::uint32_t>(extensions.size()),
        .ppEnabledExtensionNames = extensions.data()
    };

    const VpInstanceCreateInfo vpCreateInfo{
        .pCreateInfo             = &instanceInfo,
        .enabledFullProfileCount = 1,
        .pEnabledFullProfiles    = &vulkanProfile
    };

    VkInstance rawInstance{};
    VK_TRY(vpCreateInstance(_capabilities->handle(), &vpCreateInfo, nullptr, &rawInstance));

    _instance = vk::Instance(rawInstance);

    return {};
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

Expected<void> VulkanInstance::setupDebugMessenger() {
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

    VK_CREATE(
        _debugMessenger,
        _instance.createDebugUtilsMessengerEXT(debugUtilsMessengerInfo, nullptr, _dispatchLoader)
    );

    return {};
}

#endif
