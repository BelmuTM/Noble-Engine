#pragma once
#ifndef NOBLEENGINE_VULKANSWAPCHAIN_H
#define NOBLEENGINE_VULKANSWAPCHAIN_H

#include "core/platform/Platform.h"

#include "VulkanDevice.h"
#include "graphics/vulkan/common/VulkanHeader.h"

#include <string>
#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain()  = default;
    ~VulkanSwapchain() = default;

    VulkanSwapchain(const VulkanSwapchain&)            = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;
    VulkanSwapchain(VulkanSwapchain&&)                 = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&)      = delete;

    [[nodiscard]] bool create(
        const Platform::Window& window, const VulkanDevice& device, vk::SurfaceKHR surface, std::string& errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool recreate(vk::SurfaceKHR surface, std::string& errorMessage);

    [[nodiscard]] const vk::SwapchainKHR& handle() const { return _swapchain; }

    [[nodiscard]] std::vector<vk::Image> getImages() const { return _images; }
    [[nodiscard]] std::vector<vk::ImageView> getImageViews() const { return _imageViews; }

    [[nodiscard]] vk::Format getFormat() const { return _format; }
    [[nodiscard]] vk::Extent2D getExtent() const { return _extent; }

    [[nodiscard]] float getAspectRatio() const {
        return static_cast<float>(_extent.width) / static_cast<float>(_extent.height);
    }

private:
    struct SwapchainSupportInfo {
        vk::SurfaceCapabilitiesKHR        capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR>   presentModes;
    };

    const Platform::Window* _window = nullptr;
    const VulkanDevice*     _device = nullptr;

    vk::SwapchainKHR           _swapchain{};
    std::vector<vk::Image>     _images{};
    std::vector<vk::ImageView> _imageViews{};

    vk::Format   _format = vk::Format::eUndefined;
    vk::Extent2D _extent{};

    bool createSwapchain(vk::SurfaceKHR surface, std::string& errorMessage);
    bool createImageViews(std::string& errorMessage);

    static SwapchainSupportInfo querySwapchainSupport(
        vk::PhysicalDevice device, vk::SurfaceKHR _surface, std::string& errorMessage
    );

    static vk::SurfaceFormatKHR chooseSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);
    static vk::PresentModeKHR   choosePresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);
    [[nodiscard]] vk::Extent2D  chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;
};

#endif //NOBLEENGINE_VULKANSWAPCHAIN_H
