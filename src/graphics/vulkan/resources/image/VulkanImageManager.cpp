#include "VulkanImageManager.h"

#include "core/ResourceManager.h"
#include "core/debug/ErrorHandling.h"

#include "core/common/StbUsage.h"

bool VulkanImageManager::create(
    const VulkanDevice& device, const VulkanCommandManager& commandManager, std::string& errorMessage
) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return true;
}

void VulkanImageManager::destroy() noexcept {
    for (auto& image : _images) {
        image.destroy(*_device);
    }

    _images.clear();

    _device         = nullptr;
    _commandManager = nullptr;
}

bool VulkanImageManager::createDefaultTexture(VulkanImage& texture, std::string& errorMessage) {
    constexpr auto    extent          = vk::Extent3D{1, 1, 1};
    constexpr auto    format          = vk::Format::eR8G8B8A8Srgb;
    constexpr uint8_t channels        = 4;
    constexpr uint8_t bytesPerChannel = 1;

    static constexpr uint32_t blackPixel = 0xFF000000;

    TRY(texture.createFromData(
        &blackPixel, channels, bytesPerChannel, extent, format, _device, _commandManager, errorMessage
    ));

    addImage(texture);

    return true;
}

bool VulkanImageManager::loadTextureFromFile(VulkanImage& texture, const std::string& path, std::string& errorMessage) {
    constexpr int depth = 1;

    const std::string fullPath = textureFilesPath + path;

    int width, height, channels;
    stbi_uc* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        errorMessage = "Failed to load texture \"" + fullPath + "\"";
        return false;
    }

    const auto extent = vk::Extent3D{
        static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth)
    };

    constexpr auto    format          = vk::Format::eR8G8B8A8Srgb;
    constexpr uint8_t bytesPerChannel = 1;

    TRY(texture.createFromData(
        pixels, STBI_rgb_alpha, bytesPerChannel, extent, format, _device, _commandManager, errorMessage
    ));

    stbi_image_free(pixels);

    addImage(texture);

    return true;
}

bool VulkanImageManager::createColorBuffer(
    VulkanImage& colorBuffer, const vk::Extent2D extent, const vk::Format format, std::string& errorMessage
) const {
    const auto colorExtent = vk::Extent3D(extent.width, extent.height, 1);

    colorBuffer.setFormat(format);
    colorBuffer.setExtent(colorExtent);

    TRY(colorBuffer.createImage(
        colorExtent,
        vk::ImageType::e2D,
        format,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(colorBuffer.createImageView(
        vk::ImageViewType::e2D, format, vk::ImageAspectFlagBits::eColor, _device, errorMessage
    ));

    TRY(colorBuffer.transitionImageLayout(
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal,
        _commandManager,
        errorMessage
    ));

    return true;
}

bool VulkanImageManager::createDepthBuffer(
    VulkanImage& depthBuffer, const vk::Extent2D extent, std::string& errorMessage
) const {
    constexpr auto depthFormat = vk::Format::eD32Sfloat;
    const     auto depthExtent = vk::Extent3D(extent.width, extent.height, 1);

    depthBuffer.setFormat(depthFormat);
    depthBuffer.setExtent(depthExtent);

    vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eDepth;
    if (VulkanImage::hasStencilComponent(depthBuffer.getFormat())) {
        aspects |= vk::ImageAspectFlagBits::eStencil;
    }

    TRY(depthBuffer.createImage(
        depthExtent,
        vk::ImageType::e2D,
        depthFormat,
        vk::ImageUsageFlagBits::eDepthStencilAttachment,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(depthBuffer.createImageView(vk::ImageViewType::e2D, depthFormat, aspects, _device, errorMessage));

    TRY(depthBuffer.transitionImageLayout(
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal,
        _commandManager,
        errorMessage
    ));

    return true;
}
