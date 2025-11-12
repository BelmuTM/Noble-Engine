#include "VulkanCapabilities.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

bool VulkanCapabilities::create(std::string& errorMessage) {
    VpCapabilitiesCreateInfo createInfo{};
    createInfo.apiVersion       = VULKAN_VERSION;
    createInfo.flags            = VP_PROFILE_CREATE_STATIC_BIT;
    createInfo.pVulkanFunctions = nullptr;

    VK_TRY(vpCreateCapabilities(&createInfo, nullptr, &_capabilities), errorMessage);

    return true;
}

void VulkanCapabilities::destroy() {
    if (_capabilities) {
        vpDestroyCapabilities(_capabilities, nullptr);
        _capabilities = VK_NULL_HANDLE;
    }
}
