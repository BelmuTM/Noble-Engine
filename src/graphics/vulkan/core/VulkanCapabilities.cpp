#include "VulkanCapabilities.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

Expected<void> VulkanCapabilities::create() noexcept {
    constexpr VpCapabilitiesCreateInfo createInfo{
        .flags            = VP_PROFILE_CREATE_STATIC_BIT,
        .apiVersion       = VULKAN_VERSION,
        .pVulkanFunctions = nullptr
    };

    VK_TRY(vpCreateCapabilities(&createInfo, nullptr, &_capabilities));

    return {};
}

void VulkanCapabilities::destroy() noexcept {
    if (_capabilities) {
        vpDestroyCapabilities(_capabilities, nullptr);
        _capabilities = VK_NULL_HANDLE;
    }
}
