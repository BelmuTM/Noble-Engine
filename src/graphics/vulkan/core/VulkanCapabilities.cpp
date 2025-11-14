#include "VulkanCapabilities.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanCapabilities::create(std::string& errorMessage) {
    constexpr VpCapabilitiesCreateInfo createInfo{
        .flags            = VP_PROFILE_CREATE_STATIC_BIT,
        .apiVersion       = VULKAN_VERSION,
        .pVulkanFunctions = nullptr
    };

    VK_TRY(vpCreateCapabilities(&createInfo, nullptr, &_capabilities), errorMessage);

    return true;
}

void VulkanCapabilities::destroy() {
    if (_capabilities) {
        vpDestroyCapabilities(_capabilities, nullptr);
        _capabilities = VK_NULL_HANDLE;
    }
}
