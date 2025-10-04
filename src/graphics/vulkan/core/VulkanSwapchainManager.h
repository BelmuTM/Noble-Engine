#pragma once
#ifndef NOBLEENGINE_VULKANSWAPCHAINMANAGER_H
#define NOBLEENGINE_VULKANSWAPCHAINMANAGER_H

#include "core/platform/Platform.h"

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
        VulkanContext&     context,
        Platform::Window&  window,
        uint32_t           framesInFlight,
        uint32_t           swapchainImageCount,
        std::string&       errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool handleFramebufferResize(std::string& errorMessage);

    [[nodiscard]] bool recreateSwapchain(std::string& errorMessage);

    std::optional<uint32_t> acquireNextImage(uint32_t frameIndex, std::string& errorMessage, bool& discardLogging);

    [[nodiscard]] bool submitCommandBuffer(
        const VulkanCommandManager& commandManager,
        uint32_t                    frameIndex,
        uint32_t                    imageIndex,
        std::string&                errorMessage,
        bool&                       discardLogging
    );

private:
    VulkanContext*    _context = nullptr;
    Platform::Window* _window  = nullptr;

    VulkanSyncObjects _syncObjects;

    uint32_t _framesInFlight = 0;

    [[nodiscard]] bool waitForImageFence(uint32_t frameIndex, uint32_t imageIndex, std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANSWAPCHAINMANAGER_H