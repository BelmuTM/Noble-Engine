#pragma once

#include "core/debug/ErrorHandling.h"
#include "core/platform/Window.h"

#include "graphics/vulkan/core/VulkanContext.h"
#include "graphics/vulkan/core/VulkanSynchronization.h"

class VulkanSwapchainManager {
public:
    VulkanSwapchainManager()  = default;
    ~VulkanSwapchainManager() = default;

    VulkanSwapchainManager(const VulkanSwapchainManager&)            = delete;
    VulkanSwapchainManager& operator=(const VulkanSwapchainManager&) = delete;

    VulkanSwapchainManager(VulkanSwapchainManager&&)            = delete;
    VulkanSwapchainManager& operator=(VulkanSwapchainManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        Window&              window,
        const VulkanSurface& surface,
        const VulkanDevice&  device,
        VulkanSwapchain&     swapchain,
        std::uint32_t        framesInFlight
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> recreateSwapchain();

    [[nodiscard]] Expected<VulkanSwapchain::SwapchainOp<uint32_t>> acquireNextImage(::uint32_t frameIndex);

    [[nodiscard]] Expected<VulkanSwapchain::SwapchainOpVoid> submitCommandBuffer(
        vk::CommandBuffer commandBuffer,
        std::uint32_t     frameIndex,
        std::uint32_t     imageIndex
    ) const;

private:
    [[nodiscard]] Expected<void> waitForImageFence(std::uint32_t frameIndex, std::uint32_t imageIndex);

    Window* _window = nullptr;

    const VulkanSurface* _surface   = nullptr;
    const VulkanDevice*  _device    = nullptr;
    VulkanSwapchain*     _swapchain = nullptr;

    VulkanSynchronization _syncObjects{};

    std::uint32_t _framesInFlight = 0;
};
