#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanCapabilities.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

class VulkanDevice {
public:
    VulkanDevice()  = default;
    ~VulkanDevice() = default;

    VulkanDevice(const VulkanDevice&)            = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    VulkanDevice(VulkanDevice&&)            = delete;
    VulkanDevice& operator=(VulkanDevice&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanCapabilities& capabilities,
        const vk::Instance&       instance,
        const vk::SurfaceKHR&     surface
    ) noexcept;

    void destroy() noexcept;

    struct QueueFamilyIndices {
        std::uint32_t graphicsFamily = UINT32_MAX;
        std::uint32_t presentFamily  = UINT32_MAX;

        [[nodiscard]] bool isComplete() const noexcept {
            return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
        }
    };

    [[nodiscard]] vk::PhysicalDevice getPhysicalDevice() const noexcept { return _physicalDevice; }
    [[nodiscard]] const vk::Device& getLogicalDevice() const noexcept { return _logicalDevice; }

    [[nodiscard]] vk::PhysicalDeviceLimits getLimits() const noexcept {
        return _physicalDevice.getProperties2().properties.limits;
    }

    [[nodiscard]] VmaAllocator getAllocator() const noexcept { return _allocator; }

    [[nodiscard]] const QueueFamilyIndices& getQueueFamilyIndices() const noexcept { return _queueFamilyIndices; }

    [[nodiscard]] vk::Queue getGraphicsQueue() const noexcept { return _graphicsQueue; }
    [[nodiscard]] vk::Queue getPresentQueue() const noexcept { return _presentQueue; }

    [[nodiscard]] vk::QueryPool getQueryPool() const noexcept { return _queryPool; }

private:
    static bool isPhysicalDeviceSuitable(vk::PhysicalDevice device);

    Expected<void> pickPhysicalDevice();

    static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);

    Expected<void> createLogicalDevice(QueueFamilyIndices queueFamilyIndices);

    Expected<void> createAllocator();

    Expected<void> createQueryPool();

    const VulkanCapabilities* _capabilities = nullptr;

    vk::Instance _instance{};

    vk::PhysicalDevice _physicalDevice{};
    vk::Device         _logicalDevice{};

    VmaAllocator _allocator;

    QueueFamilyIndices _queueFamilyIndices{};

    vk::Queue _graphicsQueue{};
    vk::Queue _presentQueue{};

    vk::QueryPool _queryPool{};
};
