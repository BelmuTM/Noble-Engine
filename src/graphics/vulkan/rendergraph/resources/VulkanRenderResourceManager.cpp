#include "VulkanRenderResourceManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

#include <ranges>

Expected<void> VulkanRenderResourceManager::create(
    const VulkanDevice&         device,
    const VulkanSwapchain&      swapchain,
    const VulkanCommandManager& commandManager,
    const std::uint32_t         framesInFlight
) noexcept {
    _device         = &device;
    _swapchain      = &swapchain;
    _commandManager = &commandManager;
    _framesInFlight = framesInFlight;

    return {};
}

void VulkanRenderResourceManager::destroy() noexcept {
    _resources.clear();
    _resourceReaders.clear();
    _resourceWriters.clear();

    for (const auto& resourceImage : _resourceImages | std::views::values) {
        if (resourceImage) {
            resourceImage->destroy();
        }
    }

    _resourceImages.clear();

    _device         = nullptr;
    _swapchain      = nullptr;
    _commandManager = nullptr;
}

Expected<void> VulkanRenderResourceManager::recreate(VulkanRenderGraph& renderGraph) {
    // Recreate resource images
    for (const auto& resourceImage : _resourceImages | std::views::values) {
        if (!resourceImage) continue;

        resourceImage->destroy();

        TRY(createResourceImage(
            *resourceImage, resourceImage->getFormat(), _swapchain->getExtent(), _device, _commandManager
        ));
    }

    // Re-bind descriptor sets
    rebindDescriptors(renderGraph);

    return {};
}

Expected<void> VulkanRenderResourceManager::createResource(const VulkanRenderPassResourceDescriptor& descriptor) {
    auto [cachedResource, inserted] = _resourceImages.emplace(descriptor.name, std::make_unique<VulkanImage>());

    TRY_CATCH(
        createResourceImage(
            *cachedResource->second, descriptor.format, _swapchain->getExtent(), _device, _commandManager
        ),
        _resourceImages.erase(cachedResource)
    );

    VulkanRenderPassResource resource(descriptor);
    resource.setImage(cachedResource->second.get());

    addResource(resource);

    return {};
}

Expected<void> VulkanRenderResourceManager::createResourceImage(
    VulkanImage&                resourceImage,
    const vk::Format            format,
    const vk::Extent2D          extent,
    const VulkanDevice*         device,
    const VulkanCommandManager* commandManager
) {
    bool isDepth = VulkanImage::isDepthBuffer(format);

    const auto extent3D = vk::Extent3D(extent.width, extent.height, 1);

    vk::ImageUsageFlags usageFlags = isDepth
        ? vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled
        : vk::ImageUsageFlagBits::eColorAttachment        | vk::ImageUsageFlagBits::eSampled;

    vk::ImageAspectFlags aspectFlags = isDepth
        ? vk::ImageAspectFlagBits::eDepth
        : vk::ImageAspectFlagBits::eColor;

    vk::ImageLayout targetLayout = isDepth
        ? vk::ImageLayout::eDepthStencilAttachmentOptimal
        : vk::ImageLayout::eColorAttachmentOptimal;

    if (isDepth && VulkanImage::hasStencilComponent(format)) {
        aspectFlags |= vk::ImageAspectFlagBits::eStencil;
    }

    resourceImage
        .setFormat(format)
        .setExtent(extent3D)
        .setUsageFlags(usageFlags)
        .setAspectFlags(aspectFlags)
        .setDescriptorType(vk::DescriptorType::eCombinedImageSampler);

    TRY(resourceImage.createImage(
        vk::ImageType::e2D, format, extent3D, 1, usageFlags, VMA_MEMORY_USAGE_GPU_ONLY, device
    ));

    TRY(resourceImage.createImageView(vk::ImageViewType::e2D, format, aspectFlags, 1, device));

    TRY(resourceImage.transitionLayout(commandManager, vk::ImageLayout::eUndefined, targetLayout));

    TRY(resourceImage.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, device));

    return {};
}

Expected<void> VulkanRenderResourceManager::allocateDescriptors(VulkanRenderPass* pass) {
    // Allocate per-pass descriptor sets keyed by set index
    for (const auto& [set, scheme] : pass->getShaderProgram()->getDescriptorSchemes()) {

        // Create one manager (pool, layout) per descriptor scheme
        auto descriptorManager = std::make_unique<VulkanDescriptorManager>();
        TRY(descriptorManager->create(_device->getLogicalDevice(), scheme, _framesInFlight, 1));

        VulkanDescriptorSets* descriptorSets = nullptr;
        TRY(descriptorManager->allocate(descriptorSets));

        // Store descriptor manager and sets keyed by set index
        pass->getDescriptorManagers()[set] = std::move(descriptorManager);
        pass->getDescriptorSets()[set]     = descriptorSets;

        bindDescriptors(descriptorSets, scheme);
    }

    return {};
}

void VulkanRenderResourceManager::bindDescriptors(
    const VulkanDescriptorSets* descriptorSets, const VulkanDescriptorScheme& scheme
) {
    for (const auto& descriptor : scheme) {
        const VulkanRenderPassResource* resource = getResource(descriptor.name);
        if (!resource || !resource->image) continue;

        // Bind resource for each frame in flight
        descriptorSets->updatePerFrameDescriptorSets(resource->image->getDescriptorInfo(descriptor.binding));
    }
}

void VulkanRenderResourceManager::rebindDescriptors(VulkanRenderGraph& renderGraph) {
    for (const auto& pass : renderGraph.getPasses()) {

        for (const auto& [set, scheme] : pass->getShaderProgram()->getDescriptorSchemes()) {
            auto cachedSets = pass->getDescriptorSets().find(set);
            if (cachedSets == pass->getDescriptorSets().end()) continue;

            const VulkanDescriptorSets* descriptorSets = cachedSets->second;
            if (!descriptorSets) continue;

            bindDescriptors(descriptorSets, scheme);
        }

    }
}
