#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanCapabilities {
public:
    VulkanCapabilities()  = default;
    ~VulkanCapabilities() = default;

    VulkanCapabilities(const VulkanCapabilities&)            = delete;
    VulkanCapabilities& operator=(const VulkanCapabilities&) = delete;

    VulkanCapabilities(VulkanCapabilities&&)            = delete;
    VulkanCapabilities& operator=(VulkanCapabilities&&) = delete;

    [[nodiscard]] Expected<void> create() noexcept;

    void destroy() noexcept;

    [[nodiscard]] VpCapabilities handle() const noexcept { return _capabilities; }

private:
    VpCapabilities _capabilities = VK_NULL_HANDLE;
};
