#pragma once
#ifndef NOBLEENGINE_VULKANCAPABILITIES_H
#define NOBLEENGINE_VULKANCAPABILITIES_H

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanCapabilities {
public:
    VulkanCapabilities()  = default;
    ~VulkanCapabilities() = default;

    // Implicit conversion operator
    operator VpCapabilities() const noexcept { return _capabilities; }

    VulkanCapabilities(const VulkanCapabilities&)            = delete;
    VulkanCapabilities& operator=(const VulkanCapabilities&) = delete;

    VulkanCapabilities(VulkanCapabilities&&)            = delete;
    VulkanCapabilities& operator=(VulkanCapabilities&&) = delete;

    [[nodiscard]] bool create(std::string& errorMessage);
    void destroy();

private:
    VpCapabilities _capabilities = VK_NULL_HANDLE;
};

#endif // NOBLEENGINE_VULKANCAPABILITIES_H
