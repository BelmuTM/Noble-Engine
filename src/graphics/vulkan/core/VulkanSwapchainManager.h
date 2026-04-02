#pragma once

#include "core/platform/Window.h"

#include "VulkanContext.h"
#include "VulkanSyncObjects.h"

class VulkanSwapchainManager {
public:
    VulkanSwapchainManager()  = default;
    ~VulkanSwapchainManager() = default;

    VulkanSwapchainManager(const VulkanSwapchainManager&)            = delete;
    VulkanSwapchainManager& operator=(const VulkanSwapchainManager&) = delete;

    VulkanSwapchainManager(VulkanSwapchainManager&&)            = delete;
    VulkanSwapchainManager& operator=(VulkanSwapchainManager&&) = delete;

    [[nodiscard]] bool create(
        Window&              window,
        const VulkanSurface& surface,
        const VulkanDevice&  device,
        VulkanSwapchain&     swapchain,
        std::uint32_t        framesInFlight,
        std::string&         errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool recreateSwapchain(std::string& errorMessage);

    [[nodiscard]] bool acquireNextImage(
        std::uint32_t& imageIndex, std::uint32_t frameIndex, std::string& errorMessage, bool& discardLogging
    );

    [[nodiscard]] bool submitCommandBuffer(
        vk::CommandBuffer commandBuffer,
        std::uint32_t     frameIndex,
        std::uint32_t     imageIndex,
        std::string&      errorMessage,
        bool&             discardLogging
    );

    [[nodiscard]] bool isOutOfDate() const noexcept { return _outOfDate; }

private:
    [[nodiscard]] bool waitForImageFence(std::uint32_t frameIndex, std::uint32_t imageIndex, std::string& errorMessage);

    Window* _window = nullptr;

    const VulkanSurface* _surface   = nullptr;
    const VulkanDevice*  _device    = nullptr;
    VulkanSwapchain*     _swapchain = nullptr;

    VulkanSyncObjects _syncObjects{};

    std::uint32_t _framesInFlight = 0;

    bool _outOfDate = false;
};
