#include "VulkanImageManager.h"

#include "graphics/vulkan/resources/StbUsage.h"

#include "core/debug/ErrorHandling.h"
#include "core/ResourceManager.h"

bool VulkanImageManager::create(
    const VulkanDevice&         device,
    const VulkanCommandManager& commandManager,
    std::string&                errorMessage) noexcept {
    _device         = &device;
    _commandManager = &commandManager;

    return true;
}

void VulkanImageManager::destroy() noexcept {
    for (auto& image : images) {
        image.destroy(*_device);
    }
    images.clear();

    _device         = nullptr;
    _commandManager = nullptr;
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

bool VulkanImageManager::loadTextureFromFile(VulkanImage& texture, const std::string& path, std::string& errorMessage) {
    constexpr int depth = 1;

    const std::string fullPath = textureFilesPath + path;

    int width, height, channels;
    stbi_uc* pixels = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

    if (!pixels) {
        errorMessage = "Failed to load texture \"" + fullPath + "\"";
        return false;
    }

    constexpr auto format = vk::Format::eR8G8B8A8Srgb;

    const auto extent = vk::Extent3D{
        static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(depth)
    };

    const vk::DeviceSize textureSize = width * height * STBI_rgb_alpha;

    VulkanBuffer stagingBuffer;

    TRY(stagingBuffer.create(
        textureSize,
        vk::BufferUsageFlagBits::eTransferSrc,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        _device,
        errorMessage
    ));

    void* stagingData = stagingBuffer.mapMemory(errorMessage);
    if (!stagingData) return false;

    memcpy(stagingData, pixels, textureSize);
    stagingBuffer.unmapMemory();

    stbi_image_free(pixels);

    TRY(texture.createImage(
        extent,
        vk::ImageType::e2D,
        format,
        vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        _device,
        errorMessage
    ));

    TRY(texture.transitionImageLayout(
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eTransferDstOptimal,
        _commandManager,
        errorMessage
    ));

    TRY(texture.copyBufferToImage(stagingBuffer, extent, _commandManager, errorMessage));

    TRY(texture.transitionImageLayout(
        vk::ImageLayout::eTransferDstOptimal,
        vk::ImageLayout::eShaderReadOnlyOptimal,
        _commandManager,
        errorMessage
    ));

    TRY(texture.createImageView(
        vk::ImageViewType::e2D,
        format,
        vk::ImageAspectFlagBits::eColor,
        _device,
        errorMessage
    ));

    TRY(texture.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, _device, errorMessage));

    stagingBuffer.destroy();

    addImage(texture);

    return true;
}
