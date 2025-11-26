#include "VulkanImageManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanImageManager::create(
    const VulkanDevice& device, const VulkanCommandManager& commandManager, std::string& errorMessage
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return true;
}

void VulkanImageManager::destroy() noexcept {
    for (const auto& image : _images) {
        image->destroy(*_device);
    }

    _images.clear();

    _device         = nullptr;
    _commandManager = nullptr;
}

bool VulkanImageManager::loadImage(
    VulkanImage*& image, const Image* imageData, const bool useMipmaps, std::string& errorMessage
) {
    if (!imageData) {
        errorMessage = "Failed to load Vulkan image: data is null";
        return false;
    }

    VulkanImage tempImage{};

    constexpr int depth = 1;

    const auto extent = vk::Extent3D{
        static_cast<uint32_t>(imageData->width),
        static_cast<uint32_t>(imageData->height),
        static_cast<uint32_t>(depth)
    };

    constexpr auto    format          = vk::Format::eR8G8B8A8Srgb;
    constexpr uint8_t bytesPerChannel = 1;

    const uint32_t mipLevels = useMipmaps ? getMipLevels(extent) : 1;

    TRY(tempImage.createFromData(
        imageData->pixels.data(),
        imageData->channels,
        bytesPerChannel,
        format,
        extent,
        mipLevels,
        _device,
        _commandManager,
        errorMessage
    ));

    _images.push_back(std::make_unique<VulkanImage>(std::move(tempImage)));
    image = _images.back().get();

    return true;
}

bool VulkanImageManager::createColorBuffer(
    VulkanImage& colorBuffer, const vk::Format format, const vk::Extent2D extent, std::string& errorMessage
) const {
    const auto colorExtent = vk::Extent3D{extent.width, extent.height, 1};

    colorBuffer.setFormat(format);
    colorBuffer.setExtent(colorExtent);
    colorBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    TRY(colorBuffer.createImage(
        vk::ImageType::e2D,
        format,
        colorExtent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(colorBuffer.createImageView(
        vk::ImageViewType::e2D, format, vk::ImageAspectFlagBits::eColor, 1, _device, errorMessage
    ));

    TRY(colorBuffer.transitionLayout(
        _commandManager, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    TRY(colorBuffer.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, _device, errorMessage));

    return true;
}

bool VulkanImageManager::createDepthBuffer(
    VulkanImage& depthBuffer, const vk::Extent2D extent, std::string& errorMessage
) const {
    constexpr auto depthFormat = vk::Format::eD32Sfloat;
    const     auto depthExtent = vk::Extent3D(extent.width, extent.height, 1);

    depthBuffer.setFormat(depthFormat);
    depthBuffer.setExtent(depthExtent);
    depthBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eDepth;
    if (VulkanImage::hasStencilComponent(depthBuffer.getFormat())) {
        aspects |= vk::ImageAspectFlagBits::eStencil;
    }

    TRY(depthBuffer.createImage(
        vk::ImageType::e2D,
        depthFormat,
        depthExtent,
        1,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(depthBuffer.createImageView(vk::ImageViewType::e2D, depthFormat, aspects, 1, _device, errorMessage));

    TRY(depthBuffer.transitionLayout(
        _commandManager, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    ));

    TRY(depthBuffer.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, _device, errorMessage));

    return true;
}
