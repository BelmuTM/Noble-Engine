#pragma once
#ifndef NOBLEENGINE_VULKANDEVICE_H
#define NOBLEENGINE_VULKANDEVICE_H

#include <string>

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

class VulkanDevice {
public:
    VulkanDevice() = default;
    ~VulkanDevice() = default;

    VulkanDevice(const VulkanDevice&)            = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;
    VulkanDevice(VulkanDevice&&)                 = delete;
    VulkanDevice& operator=(VulkanDevice&&)      = delete;

    [[nodiscard]] bool create(vk::Instance instance, vk::SurfaceKHR surface, std::string& errorMessage) noexcept;
    void               destroy() noexcept;

    struct QueueFamilyIndices {
        uint32_t graphicsFamily = UINT32_MAX;
        uint32_t presentFamily  = UINT32_MAX;

        [[nodiscard]] bool isComplete() const {
            return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
        }
    };

    [[nodiscard]] vk::PhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    [[nodiscard]] vk::Device         getLogicalDevice () const { return logicalDevice ; }

    [[nodiscard]] const QueueFamilyIndices& getQueueFamilyIndices() const { return _queueFamilyIndices; }

private:
    vk::PhysicalDevice physicalDevice{};
    vk::Device         logicalDevice {};

    QueueFamilyIndices _queueFamilyIndices;

    vk::Queue graphicsQueue{};
    vk::Queue presentQueue {};

    static bool isPhysicalDeviceSuitable(vk::PhysicalDevice device);
    bool        pickPhysicalDevice(vk::Instance instance, std::string& errorMessage);

    static QueueFamilyIndices findQueueFamilies(vk::PhysicalDevice device, vk::SurfaceKHR surface);
    bool                      createLogicalDevice(QueueFamilyIndices queueFamilyIndices, std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANDEVICE_H
