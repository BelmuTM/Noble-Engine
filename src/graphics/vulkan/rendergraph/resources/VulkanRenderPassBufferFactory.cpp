#include "VulkanRenderPassBufferFactory.h"

#include "core/debug/ErrorHandling.h"

bool VulkanRenderPassBufferFactory::createDepthBufferImage(
    VulkanImage&                depthBuffer,
    const vk::Format            format,
    const vk::Extent2D          extent,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    const auto depthExtent = vk::Extent3D(extent.width, extent.height, 1);

    depthBuffer.setFormat(format);
    depthBuffer.setExtent(depthExtent);
    depthBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eDepth;
    if (VulkanImage::hasStencilComponent(depthBuffer.getFormat())) {
        aspects |= vk::ImageAspectFlagBits::eStencil;
    }

    TRY_deprecated(depthBuffer.createImage(
        vk::ImageType::e2D,
        format,
        depthExtent,
        1,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        device,
        errorMessage
    ));

    TRY_deprecated(depthBuffer.createImageView(vk::ImageViewType::e2D, format, aspects, 1, device, errorMessage));

    TRY_deprecated(depthBuffer.transitionLayout(
        commandManager, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    ));

    TRY_deprecated(depthBuffer.createSampler(
        vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, device, errorMessage
    ));

    return true;
}

bool VulkanRenderPassBufferFactory::createColorBufferImage(
    VulkanImage&                colorBuffer,
    const vk::Format            format,
    const vk::Extent2D          extent,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager,
    std::string&                errorMessage
) {
    const auto colorExtent = vk::Extent3D{extent.width, extent.height, 1};

    colorBuffer.setFormat(format);
    colorBuffer.setExtent(colorExtent);
    colorBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    TRY_deprecated(colorBuffer.createImage(
        vk::ImageType::e2D,
        format,
        colorExtent,
        1,
        vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        device,
        errorMessage
    ));

    TRY_deprecated(colorBuffer.createImageView(
        vk::ImageViewType::e2D, format, vk::ImageAspectFlagBits::eColor, 1, device, errorMessage
    ));

    TRY_deprecated(colorBuffer.transitionLayout(
        commandManager, errorMessage,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    TRY_deprecated(colorBuffer.createSampler(
        vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, device, errorMessage
    ));

    return true;
}
