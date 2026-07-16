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

Expected<void> VulkanRenderResourceManager::createResource(const VulkanPassResourceDescriptor& descriptor) {
    auto [cachedResource, inserted] = _resourceImages.emplace(descriptor.name, std::make_unique<VulkanImage>());

    TRY_CATCH(
        createResourceImage(
            *cachedResource->second, descriptor.format, _swapchain->getExtent(), _device, _commandManager
        ),
        _resourceImages.erase(cachedResource)
    );

    VulkanPassResource resource(descriptor);
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

Expected<void> VulkanRenderResourceManager::allocateDescriptors(VulkanPass* pass) {
    const auto& reads = pass->getPassDescriptor().readDescriptors;
    if (reads.empty()) return {};

    // Building the descriptor scheme for the resources read in this pass
    VulkanDescriptorScheme scheme{};
    scheme.reserve(reads.size());

    for (std::size_t i = 0; i < reads.size(); i++) {
        scheme.emplace_back(i, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    }

    // Create a descriptor manager for this pass
    auto descriptorManager = std::make_unique<VulkanDescriptorManager>();
    TRY(descriptorManager->create(_device->getLogicalDevice(), scheme, _framesInFlight, 1));

    VulkanDescriptorSets* descriptorSets = nullptr;
    TRY(descriptorManager->allocate(descriptorSets));

    bindDescriptors(descriptorSets, reads);

    pass->setDescriptorManager(std::move(descriptorManager));
    pass->setDescriptorSets(descriptorSets);

    return {};
}

void VulkanRenderResourceManager::bindDescriptors(
    const VulkanDescriptorSets* descriptorSets, const std::vector<VulkanGraphicsPassAttachmentDescriptor>& reads
) {
    for (std::uint32_t i = 0; i < reads.size(); i++) {
        const VulkanPassResource* resource = getResource(reads[i].name);
        if (!resource || !resource->image) continue;

        // Bind resource for each frame in flight
        descriptorSets->updatePerFrameDescriptorSets(resource->image->getDescriptorInfo(i));
    }
}

void VulkanRenderResourceManager::rebindDescriptors(VulkanRenderGraph& renderGraph) {
    for (const auto& pass : renderGraph.getPasses()) {
        const auto& reads = pass->base().getPassDescriptor().readDescriptors;
        if (reads.empty()) continue;

        if (const VulkanDescriptorSets* sets = pass->base().getDescriptorSets()) {
            bindDescriptors(sets, reads);
        }
    }
}
