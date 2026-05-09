#pragma once

#include "core/debug/ErrorHandling.h"
#include "core/platform/Window.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <optional>
#include <vector>

class VulkanSwapchain {
public:
    enum class Status : std::uint8_t {
        Success,
        Suboptimal,
        OutOfDate
    };

    template<typename T>
    struct SwapchainOp {
        Status status;
        std::optional<T> value;
    };

    struct SwapchainOpVoid {
        Status status;
    };

    VulkanSwapchain()  = default;
    ~VulkanSwapchain() = default;

    VulkanSwapchain(const VulkanSwapchain&)            = delete;
    VulkanSwapchain& operator=(const VulkanSwapchain&) = delete;

    VulkanSwapchain(VulkanSwapchain&&)            = delete;
    VulkanSwapchain& operator=(VulkanSwapchain&&) = delete;

    [[nodiscard]] Expected<void> create(
        const Window& window, const VulkanDevice& device, vk::SurfaceKHR surface
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> recreate(vk::SurfaceKHR surface);

    void createImages();

    [[nodiscard]] const vk::SwapchainKHR& handle() const noexcept { return _swapchain; }

    [[nodiscard]] VulkanImage* getImage(const std::uint32_t imageIndex) const { return _images[imageIndex].get(); }

    [[nodiscard]] std::uint32_t getImageCount() const noexcept { return static_cast<std::uint32_t>(_imageHandles.size()); }

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

    Expected<void> createSwapchain(vk::SurfaceKHR surface);
    Expected<void> createImageViews();

    static Expected<SwapchainSupportInfo> querySwapchainSupport(vk::PhysicalDevice device, vk::SurfaceKHR surface);

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
