#pragma once
#ifndef NOBLEENGINE_VULKANSYNCOBJECTS_H
#define NOBLEENGINE_VULKANSYNCOBJECTS_H

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanSyncObjects {
public:
    VulkanSyncObjects()  = default;
    ~VulkanSyncObjects() = default;

    VulkanSyncObjects(const VulkanSyncObjects&)            = delete;
    VulkanSyncObjects& operator=(const VulkanSyncObjects&) = delete;

    VulkanSyncObjects(VulkanSyncObjects&&)            = delete;
    VulkanSyncObjects& operator=(VulkanSyncObjects&&) = delete;

    [[nodiscard]] bool create(
        const vk::Device& device,
        uint32_t          framesInFlight,
        uint32_t          swapchainImageCount,
        std::string&      errorMessage
    ) noexcept;

    void destroy() noexcept;

    void backup();

    [[nodiscard]] const vk::Semaphore& getImageAvailableSemaphore(const uint32_t frameIndex) const noexcept {
        return _imageAvailableSemaphores[frameIndex];
    }

    [[nodiscard]] const vk::Semaphore& getRenderFinishedSemaphore(const uint32_t frameIndex) const noexcept {
        return _renderFinishedSemaphores[frameIndex];
    }

    [[nodiscard]] const vk::Fence& getInFlightFence(const uint32_t frameIndex) const noexcept {
        return _inFlightFences[frameIndex];
    }

    [[nodiscard]] std::vector<vk::Fence>& getImagesInFlightFences() noexcept { return _imagesInFlight; }

    [[nodiscard]] const vk::Fence& getImagesInFlightFence(const uint32_t imageIndex) const noexcept {
        return _imagesInFlight[imageIndex];
    }

private:
    bool createSyncObjects(uint32_t framesInFlight, uint32_t swapchainImageCount, std::string& errorMessage);

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

#endif //NOBLEENGINE_VULKANSYNCOBJECTS_H
