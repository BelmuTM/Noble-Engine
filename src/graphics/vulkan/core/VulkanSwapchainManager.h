#pragma once
#ifndef NOBLEENGINE_VULKANSWAPCHAINMANAGER_H
#define NOBLEENGINE_VULKANSWAPCHAINMANAGER_H

#include "core/Platform.h"

#include "VulkanContext.h"
#include "VulkanCommandManager.h"
#include "VulkanSyncObjects.h"

class VulkanSwapchainManager {
public:
    VulkanSwapchainManager()  = default;
    ~VulkanSwapchainManager() = default;

    VulkanSwapchainManager(const VulkanSwapchainManager&)            = delete;
    VulkanSwapchainManager& operator=(const VulkanSwapchainManager&) = delete;
    VulkanSwapchainManager(VulkanSwapchainManager&&)                 = delete;
    VulkanSwapchainManager& operator=(VulkanSwapchainManager&&)      = delete;

    [[nodiscard]] bool create(
        Platform::Window&    window,
        const VulkanSurface& surface,
        const VulkanDevice&  device,
        VulkanSwapchain&     swapchain,
        uint32_t             framesInFlight,
        std::string&         errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool isOutOfDate() const noexcept { return _outOfDate; }

    [[nodiscard]] bool recreateSwapchain(std::string& errorMessage);

    std::optional<uint32_t> acquireNextImage(uint32_t frameIndex, std::string& errorMessage, bool& discardLogging);

    [[nodiscard]] bool submitCommandBuffer(
        vk::CommandBuffer commandBuffer,
        uint32_t          frameIndex,
        uint32_t          imageIndex,
        std::string&      errorMessage,
        bool&             discardLogging
    );

private:
    Platform::Window* _window = nullptr;

    const VulkanSurface* _surface   = nullptr;
    const VulkanDevice*  _device    = nullptr;
    VulkanSwapchain*     _swapchain = nullptr;

    VulkanSyncObjects _syncObjects{};

    uint32_t _framesInFlight = 0;

    bool _outOfDate = false;

    [[nodiscard]] bool waitForImageFence(uint32_t frameIndex, uint32_t imageIndex, std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANSWAPCHAINMANAGER_H