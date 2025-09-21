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
    VulkanSyncObjects(VulkanSyncObjects&&)                 = delete;
    VulkanSyncObjects& operator=(VulkanSyncObjects&&)      = delete;

    [[nodiscard]] bool create(
        const vk::Device& device, uint32_t framesInFlight, uint32_t swapchainImageCount, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;
    void backup();

    std::vector<vk::Semaphore> imageAvailableSemaphores{};
    std::vector<vk::Semaphore> renderFinishedSemaphores{};
    std::vector<vk::Fence>     inFlightFences{};

    std::vector<vk::Fence>     oldFences{};
    std::vector<vk::Semaphore> oldImageAvailable{};
    std::vector<vk::Semaphore> oldRenderFinished{};

    std::vector<vk::Fence> imagesInFlight{};

private:
    const vk::Device* _device = nullptr;

    bool createSyncObjects(uint32_t framesInFlight, uint32_t swapchainImageCount, std::string& errorMessage);
    void destroySyncObjects();
    void cleanupOldSyncObjects();
};

#endif //NOBLEENGINE_VULKANSYNCOBJECTS_H
