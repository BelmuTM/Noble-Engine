#pragma once

#include "core/debug/ErrorHandling.h"
#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanSynchronization {
public:
    VulkanSynchronization()  = default;
    ~VulkanSynchronization() = default;

    VulkanSynchronization(const VulkanSynchronization&)            = delete;
    VulkanSynchronization& operator=(const VulkanSynchronization&) = delete;

    VulkanSynchronization(VulkanSynchronization&&)            = delete;
    VulkanSynchronization& operator=(VulkanSynchronization&&) = delete;

    [[nodiscard]] Expected<void> create(
        const vk::Device& device,
        std::uint32_t     framesInFlight,
        std::uint32_t     swapchainImageCount
    ) noexcept;

    void destroy() noexcept;

    void backup();

    [[nodiscard]] const vk::Semaphore& getImageAvailableSemaphore(const std::uint32_t frameIndex) const noexcept {
        return _imageAvailableSemaphores[frameIndex];
    }

    [[nodiscard]] const vk::Semaphore& getRenderFinishedSemaphore(const std::uint32_t frameIndex) const noexcept {
        return _renderFinishedSemaphores[frameIndex];
    }

    [[nodiscard]] const vk::Fence& getInFlightFence(const std::uint32_t frameIndex) const noexcept {
        return _inFlightFences[frameIndex];
    }

    [[nodiscard]] std::vector<vk::Fence>& getImagesInFlightFences() noexcept { return _imagesInFlight; }

    [[nodiscard]] const vk::Fence& getImagesInFlightFence(const std::uint32_t imageIndex) const noexcept {
        return _imagesInFlight[imageIndex];
    }

private:
    Expected<void> createSyncObjects(std::uint32_t framesInFlight, std::uint32_t swapchainImageCount);

    void destroySyncObjects();

    void cleanupOldSyncObjects();

    vk::Device _device{};

    std::vector<vk::Semaphore> _imageAvailableSemaphores{};
    std::vector<vk::Semaphore> _renderFinishedSemaphores{};
    std::vector<vk::Fence>     _inFlightFences{};

    std::vector<vk::Fence> _imagesInFlight{};

    std::vector<vk::Fence>     _oldFences{};
    std::vector<vk::Semaphore> _oldImageAvailable{};
    std::vector<vk::Semaphore> _oldRenderFinished{};
};
