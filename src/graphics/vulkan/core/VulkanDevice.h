#pragma once
#ifndef NOBLEENGINE_VULKANDEVICE_H
#define NOBLEENGINE_VULKANDEVICE_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/memory/VmaUsage.h"

#include <string>

class VulkanDevice {
public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;

    VulkanDevice(const VulkanDevice&)            = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    VulkanDevice(VulkanDevice&&)            = delete;
    VulkanDevice& operator=(VulkanDevice&&) = delete;

    [[nodiscard]] bool create(
        const vk::Instance&   instance,
        const vk::SurfaceKHR& surface,
        std::string&          errorMessage
    ) noexcept;

    void destroy() noexcept;

    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily  = UINT32_MAX;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
        }
    };

    [[nodiscard]] vk::PhysicalDevice getPhysicalDevice() const { return _physicalDevice; }
    [[nodiscard]] const vk::Device& getLogicalDevice() const { return _logicalDevice; }

    [[nodiscard]] VmaAllocator getAllocator() const { return _allocator; }

    [[nodiscard]] const QueueFamilyIndices& getQueueFamilyIndices() const { return _queueFamilyIndices; }

    [[nodiscard]] vk::Queue getGraphicsQueue() const { return _graphicsQueue; }
    [[nodiscard]] vk::Queue getPresentQueue() const { return _presentQueue; }

private:
    vk::Instance _instance{};

    vk::PhysicalDevice _physicalDevice{};
    vk::Device         _logicalDevice{};

    VmaAllocator _allocator{};

    QueueFamilyIndices _queueFamilyIndices{};

    vk::Queue _graphicsQueue{};
    vk::Queue _presentQueue{};

    static bool isPhysicalDeviceSuitable(vk::PhysicalDevice device);
    bool pickPhysicalDevice(std::string& errorMessage);

    static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
    bool createLogicalDevice(QueueFamilyIndices queueFamilyIndices, std::string& errorMessage);

    bool createAllocator(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANDEVICE_H
