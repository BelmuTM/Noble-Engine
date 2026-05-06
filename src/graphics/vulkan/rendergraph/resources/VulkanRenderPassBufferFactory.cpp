#include "VulkanRenderPassBufferFactory.h"

Expected<void> VulkanRenderPassBufferFactory::createDepthBufferImage(
    VulkanImage&                depthBuffer,
    const vk::Format            format,
    const vk::Extent2D          extent,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager
) {
    const auto depthExtent = vk::Extent3D(extent.width, extent.height, 1);

    depthBuffer.setFormat(format);
    depthBuffer.setExtent(depthExtent);
    depthBuffer.setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    vk::ImageAspectFlags aspects = vk::ImageAspectFlagBits::eDepth;
    if (VulkanImage::hasStencilComponent(depthBuffer.getFormat())) {
        aspects |= vk::ImageAspectFlagBits::eStencil;
    }

    TRY(depthBuffer.createImage(
        vk::ImageType::e2D,
        format,
        depthExtent,
        1,
        vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
        VMA_MEMORY_USAGE_GPU_ONLY,
        device
    ));

    TRY(depthBuffer.createImageView(vk::ImageViewType::e2D, format, aspects, 1, device));

    TRY(depthBuffer.transitionLayout(
        commandManager,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eDepthStencilAttachmentOptimal
    ));

    TRY(depthBuffer.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, device));

    return {};
}

Expected<void> VulkanRenderPassBufferFactory::createColorBufferImage(
    VulkanImage&                colorBuffer,
    const vk::Format            format,
    const vk::Extent2D          extent,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager
) {
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
        device
    ));

    TRY(colorBuffer.createImageView(vk::ImageViewType::e2D, format, vk::ImageAspectFlagBits::eColor, 1, device));

    TRY(colorBuffer.transitionLayout(
        commandManager,
        vk::ImageLayout::eUndefined,
        vk::ImageLayout::eColorAttachmentOptimal
    ));

    TRY(colorBuffer.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eRepeat, device));

    return {};
}
