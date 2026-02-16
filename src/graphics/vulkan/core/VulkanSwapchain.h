#pragma once

#include "core/platform/Window.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanDevice.h"
#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <string>
#include <vector>

class VulkanSwapchain {
public:
    VulkanSwapchain()  = default;
    ~VulkanSwapchain() = default;

    VulkanSwapchain(const VulkanSwapchain&)            = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    VulkanSwapchain(VulkanSwapchain&&)            = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

    [[nodiscard]] bool create(
        const Window& window, const VulkanDevice& device, vk::SurfaceKHR surface, std::string& errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool recreate(vk::SurfaceKHR surface, std::string& errorMessage);

    void createImages();

    [[nodiscard]] const vk::SwapchainKHR& handle() const noexcept { return _swapchain; }

    [[nodiscard]] VulkanImage* getImage(const uint32_t imageIndex) const { return _images[imageIndex].get(); }

    [[nodiscard]] uint32_t getImageCount() const noexcept { return _imageHandles.size(); }

    [[nodiscard]] vk::Format getFormat() const noexcept { return _format; }
    [[nodiscard]] vk::Extent2D getExtent() const noexcept { return _extent; }

    [[nodiscard]] float getAspectRatio() const noexcept {
        return static_cast<float>(_extent.width) / static_cast<float>(_extent.height);
    }

private:
    struct SwapchainSupportInfo {
        vk::SurfaceCapabilitiesKHR        capabilities;
        std::vector<vk::SurfaceFormatKHR> formats;
        std::vector<vk::PresentModeKHR>   presentModes;
    };

    bool createSwapchain(vk::SurfaceKHR surface, std::string& errorMessage);
    bool createImageViews(std::string& errorMessage);

    static SwapchainSupportInfo querySwapchainSupport(
        vk::PhysicalDevice device, vk::SurfaceKHR _surface, std::string& errorMessage
    );

    [[nodiscard]] static vk::SurfaceFormatKHR chooseSurfaceFormat(
        const std::vector<vk::SurfaceFormatKHR>& availableFormats
    );

    [[nodiscard]] static vk::PresentModeKHR choosePresentMode(
        const std::vector<vk::PresentModeKHR>& availablePresentModes
    );

    [[nodiscard]] vk::Extent2D chooseExtent(const vk::SurfaceCapabilitiesKHR& capabilities) const;

    const Window*       _window = nullptr;
    const VulkanDevice* _device = nullptr;

    vk::SwapchainKHR           _swapchain{};
    std::vector<vk::Image>     _imageHandles{};
    std::vector<vk::ImageView> _imageViews{};

    vk::Extent2D _extent{};
    vk::Format   _format = vk::Format::eUndefined;

    std::vector<std::unique_ptr<VulkanImage>> _images{};
};
