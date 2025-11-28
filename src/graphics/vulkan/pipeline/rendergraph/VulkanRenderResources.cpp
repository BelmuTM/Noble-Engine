#include "VulkanRenderResources.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanRenderResources::create(
    const VulkanDevice&       device,
    const VulkanSwapchain&    swapchain,
    const VulkanImageManager& imageManager,
    const uint32_t            framesInFlight,
    std::string&              errorMessage
) noexcept {
    _device         = &device;
    _swapchain      = &swapchain;
    _imageManager   = &imageManager;
    _framesInFlight = framesInFlight;

    // Depth buffer creation
    _depthBuffer = std::make_unique<VulkanImage>();
    TRY(imageManager.createDepthBuffer(*_depthBuffer, swapchain.getExtent(), errorMessage));

    VulkanRenderPassResource depthBufferResource{};
    depthBufferResource
        .setName(DEPTH_BUFFER_NAME)
        .setType(Buffer)
        .setImage(_depthBuffer.get());

    VulkanRenderPassAttachment depthBufferAttachment{};
    depthBufferAttachment
        .setResource(depthBufferResource)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearDepthStencilValue{1.0f, 0});

    _depthBufferAttachment = std::make_unique<VulkanRenderPassAttachment>(depthBufferAttachment);

    addResource(depthBufferResource);

    return true;
}

void VulkanRenderResources::destroy() noexcept {
    // Global resources destruction
    for (const auto& descriptorManager : _descriptorManagers) {
        descriptorManager->destroy();
    }

    _resources.clear();
    _resourceReaders.clear();
    _resourceWriters.clear();

    _descriptorManagers.clear();
    _descriptorBindingsPerManager.clear();
    _descriptorSetGroups.clear();

    // Depth buffer destruction
    _depthBuffer->destroy(*_device);

    _device       = nullptr;
    _swapchain    = nullptr;
    _imageManager = nullptr;
}

[[nodiscard]] bool VulkanRenderResources::recreate(std::string& errorMessage) {
    // Depth buffer recreation
    _depthBuffer->destroy(*_device);

    TRY(_imageManager->createDepthBuffer(*_depthBuffer, _swapchain->getExtent(), errorMessage));

    // Descriptor sets re-binding
    if (!_descriptorSetGroups.empty() && !_descriptorManagers.empty()) {

        for (size_t managerIndex = 0; managerIndex < _descriptorManagers.size(); managerIndex++) {
            const VulkanDescriptorSets* descriptorSets = _descriptorSetGroups[managerIndex];

            if (!descriptorSets) continue;

            const auto& bindings = _descriptorBindingsPerManager[managerIndex];

            for (const auto& [binding, resourceName] : bindings) {
                auto it = _resources.find(resourceName);
                if (it == _resources.end()) continue;

                const auto& resource = it->second;
                if (!resource->image) continue;

                descriptorSets->bindPerFrameResource(resource->image->getDescriptorInfo(binding));
            }
        }

    }

    return true;
}

bool VulkanRenderResources::createColorAttachments(
    VulkanRenderPass*         pass,
    const VulkanImageManager& imageManager,
    VulkanFrameResources&     frameResources,
    std::string&              errorMessage
) {
    for (const auto& colorOutput : pass->getShaderProgram()->getStageOutputs()) {
        VulkanImage* colorImage = frameResources.allocateColorBuffer();

        constexpr auto format = vk::Format::eB8G8R8A8Srgb;

        TRY(imageManager.createColorBuffer(*colorImage, format, frameResources.getExtent(), errorMessage));

        VulkanRenderPassResource colorBuffer{};
        colorBuffer
            .setName(colorOutput)
            .setType(Buffer)
            .setImage(colorImage);

        VulkanRenderPassAttachment colorAttachment{};
        colorAttachment
            .setResource(colorBuffer)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});

        pass->addColorAttachment(colorAttachment);

        addResource(colorBuffer);
        addResourceWriter(colorOutput, pass);
    }

    return true;
}

bool VulkanRenderResources::allocateDescriptors(std::string& errorMessage) {
    for (auto& scheme : _descriptorSchemes | std::views::values) {
        VulkanDescriptorScheme uniqueScheme;
        uniqueScheme.reserve(scheme.size());

        std::unordered_map<uint32_t, size_t> seenDescriptor;

        for (auto& descriptor : scheme) {
            if (auto it = seenDescriptor.find(descriptor.binding); it != seenDescriptor.end()) {
                uniqueScheme[it->second] = descriptor;
            } else {
                seenDescriptor[descriptor.binding] = uniqueScheme.size();
                uniqueScheme.push_back(descriptor);
            }
        }

        auto descriptorManager = std::make_unique<VulkanDescriptorManager>();
        TRY(descriptorManager->create(_device->getLogicalDevice(), uniqueScheme, _framesInFlight, 1, errorMessage));

        VulkanDescriptorSets* descriptorSets = nullptr;
        TRY(descriptorManager->allocate(descriptorSets, errorMessage));

        const size_t managerIndex = _descriptorBindingsPerManager.size();

        _descriptorManagers.push_back(std::move(descriptorManager));
        _descriptorSetGroups.push_back(descriptorSets);
        _descriptorBindingsPerManager.emplace_back();

        for (auto& descriptor : uniqueScheme) {
            auto it = _resources.find(descriptor.name);
            if (it == _resources.end()) continue;

            const auto& resource = it->second;

            if (!resource->image) continue;

            descriptorSets->bindPerFrameResource(resource->image->getDescriptorInfo(descriptor.binding));

            _descriptorBindingsPerManager[managerIndex].push_back({descriptor.binding, descriptor.name});
        }
    }

    return true;
}

std::vector<vk::DescriptorSet> VulkanRenderResources::getFrameDescriptorSets(const uint32_t frameIndex) const {
    std::vector<vk::DescriptorSet> sets{};
    sets.reserve(_descriptorSetGroups.size());

    for (const auto& group : _descriptorSetGroups) {
        sets.push_back(group->getSets()[frameIndex]);
    }

    return sets;
}
