#include "VulkanGraphResourceManager.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

Expected<void> VulkanGraphResourceManager::create(
    const VulkanDevice& device,
    const std::uint32_t framesInFlight
) noexcept {
    _device         = &device;
    _framesInFlight = framesInFlight;

    return {};
}

void VulkanGraphResourceManager::destroy() noexcept {

    _registrations.clear();

    _resources.clear();

    for (const auto& resourceImage : _resourceImages) {
        if (resourceImage) {
            resourceImage->destroy();
        }
    }

    _resourceImages.clear();

    for (const auto& passBinding : _passBindings) {
        passBinding->manager.destroy();
    }

    _passBindings.clear();

    _device = nullptr;
}

Expected<void> VulkanGraphResourceManager::recreate(const vk::Extent2D extent) {
    // Recreate resource images
    for (const auto& resourceImage : _resourceImages) {
        if (!resourceImage) continue;

        resourceImage->destroy();

        TRY(createResourceImage(
            *resourceImage,
            resourceImage->getFormat(),
            extent,
            _device
        ));
    }

    // Re-bind descriptor sets
    rebindDescriptors();

    return {};
}

Expected<void> VulkanGraphResourceManager::createResourceImage(
    VulkanImage&        resourceImage,
    const vk::Format    format,
    const vk::Extent2D  extent,
    const VulkanDevice* device
) {
    const bool isDepth = VulkanImage::isDepthBuffer(format);

    const auto extent3D = vk::Extent3D(extent.width, extent.height, 1);

    const vk::ImageUsageFlags usageFlags = isDepth
        ? vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled
        : vk::ImageUsageFlagBits::eColorAttachment        | vk::ImageUsageFlagBits::eSampled;

    vk::ImageAspectFlags aspectFlags = isDepth
        ? vk::ImageAspectFlagBits::eDepth
        : vk::ImageAspectFlagBits::eColor;

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

    TRY(resourceImage.createSampler(vk::Filter::eLinear, vk::SamplerAddressMode::eClampToEdge, device));

    return {};
}

Expected<void> VulkanGraphResourceManager::allocateResources(const vk::Extent2D extent) {

    _resources.resize(_registrations.size());
    _resourceImages.resize(_registrations.size());

    for (VulkanGraphResourceId id = 0; id < _registrations.size(); id++) {

        const auto& [descriptor, resolver] = _registrations[id];
        auto resource = std::make_unique<VulkanGraphResource>(descriptor);

        if (resolver) {
            resource->setImageResolver(resolver);

        } else {

            auto image = std::make_unique<VulkanImage>();
            TRY(createResourceImage(*image, descriptor.format, extent, _device));

            resource->setImage(image.get());

            _resourceImages[id] = std::move(image);
        }

        _resources[id] = std::move(resource);
    }

    return {};
}

Expected<const VulkanPassDescriptorBinding*> VulkanGraphResourceManager::allocateDescriptors(
    const VulkanPassDescriptor& passDescriptor
) {

    auto passBinding = std::make_unique<VulkanPassDescriptorBinding>();

    passBinding->sampledResources = passDescriptor.getSampledResources();

    if (passBinding->sampledResources.empty()) {
        return Expected<const VulkanPassDescriptorBinding*>(nullptr);
    }

    // Building the descriptor scheme for the resources read in this pass
    VulkanDescriptorScheme scheme{};
    scheme.reserve(passBinding->sampledResources.size());

    for (std::size_t i = 0; i < passBinding->sampledResources.size(); i++) {
        scheme.emplace_back(i, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
    }

    // Create a descriptor manager for this pass and allocate its descriptor sets
    TRY(passBinding->manager.create(_device->getLogicalDevice(), scheme, _framesInFlight, 1));

    TRY_ASSIGN(passBinding->sets, passBinding->manager.allocate());

    const VulkanPassDescriptorBinding* passBindingPtr = passBinding.get();

    bindDescriptors(passBindingPtr);

    _passBindings.emplace_back(std::move(passBinding));

    return Expected(passBindingPtr);
}

void VulkanGraphResourceManager::bindDescriptors(const VulkanPassDescriptorBinding* binding) {

    for (std::uint32_t i = 0; i < binding->sampledResources.size(); i++) {

        const VulkanGraphResource* resource = getResource(binding->sampledResources[i]);
        if (!resource || !resource->image) continue;

        // Bind resource for each frame in flight
        binding->sets->updatePerFrameDescriptorSets(resource->image->getDescriptorInfo(i));
    }
}

void VulkanGraphResourceManager::rebindDescriptors() {

    for (const auto& passBinding : _passBindings) {
        bindDescriptors(passBinding.get());
    }
}
