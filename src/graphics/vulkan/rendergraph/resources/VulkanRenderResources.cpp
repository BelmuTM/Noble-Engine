#include "VulkanRenderResources.h"

#include "graphics/vulkan/rendergraph/VulkanRenderGraph.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanRenderResources::create(
    const VulkanDevice&         device,
    const VulkanSwapchain&      swapchain,
    const VulkanCommandManager& commandManager,
    const uint32_t              framesInFlight,
    std::string&                errorMessage
) noexcept {
    _device         = &device;
    _swapchain      = &swapchain;
    _commandManager = &commandManager;
    _framesInFlight = framesInFlight;

    TRY_deprecated(createDepthBuffer(errorMessage));

    return true;
}

void VulkanRenderResources::destroy() noexcept {
    _resources.clear();
    _resourceReaders.clear();
    _resourceWriters.clear();

    // Depth buffer destruction
    if (_depthBuffer) {
        _depthBuffer->destroy();
    }

    // Color buffers destruction
    for (const auto& colorBuffer : _colorBuffers) {
        if (colorBuffer) colorBuffer->destroy();
    }

    _colorBuffers.clear();

    _device         = nullptr;
    _swapchain      = nullptr;
    _commandManager = nullptr;
}

[[nodiscard]] bool VulkanRenderResources::recreate(VulkanRenderGraph& renderGraph, std::string& errorMessage) {
    // Depth buffer recreation
    if (_depthBuffer) {
        _depthBuffer->destroy();

        TRY_deprecated(_bufferFactory.createDepthBufferImage(
            *_depthBuffer, DEPTH_BUFFER_FORMAT, _swapchain->getExtent(), _device, _commandManager, errorMessage
        ));
    }

    // Color buffers recreation
    for (auto& colorBuffer : _colorBuffers) {
        colorBuffer->destroy();

        TRY_deprecated(_bufferFactory.createColorBufferImage(
            *colorBuffer, colorBuffer->getFormat(), _swapchain->getExtent(), _device, _commandManager, errorMessage
        ));

        TRY_deprecated(colorBuffer->transitionLayout(
            _commandManager, errorMessage,
            vk::ImageLayout::eShaderReadOnlyOptimal
        ));
    }

    // Re-bind descriptor sets
    rebindDescriptors(renderGraph);

    return true;
}

bool VulkanRenderResources::createDepthBuffer(std::string& errorMessage) {
    _depthBuffer = std::make_unique<VulkanImage>();

    TRY_deprecated(_bufferFactory.createDepthBufferImage(
        *_depthBuffer, DEPTH_BUFFER_FORMAT, _swapchain->getExtent(), _device, _commandManager, errorMessage
    ));

    VulkanRenderPassResource depthBufferResource{};
    depthBufferResource
        .setName(DEPTH_BUFFER_RESOURCE_NAME)
        .setType(VulkanRenderPassResourceType::Buffer)
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

bool VulkanRenderResources::createColorBuffers(
    VulkanRenderPass* pass, const VulkanFrameResources& frameResources, std::string& errorMessage
) {
    static constexpr auto format = vk::Format::eB8G8R8A8Srgb;

    for (const auto& colorOutput : pass->getShaderProgram()->getStageOutputs()) {
        _colorBuffers.push_back(std::make_unique<VulkanImage>());

        VulkanImage* colorImage = _colorBuffers.back().get();

        TRY_deprecated(_bufferFactory.createColorBufferImage(
            *colorImage, format, frameResources.getExtent(), _device, _commandManager, errorMessage
        ));

        VulkanRenderPassResource colorBuffer{};
        colorBuffer
            .setName(colorOutput)
            .setType(VulkanRenderPassResourceType::Buffer)
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

bool VulkanRenderResources::allocateDescriptors(VulkanRenderPass* pass, std::string& errorMessage) {
    // Register resource reader passes
    for (const auto& scheme : pass->getShaderProgram()->getDescriptorSchemes() | std::views::values) {
        for (const auto& descriptor : scheme) {
            if (_resources.contains(descriptor.name)) {
                addResourceReader(descriptor.name, pass);
            }
        }
    }

    // Allocate per-pass descriptor sets keyed by set index
    for (const auto& [set, scheme] : pass->getShaderProgram()->getDescriptorSchemes()) {
        // Create one manager (pool, layout) per descriptor scheme
        auto descriptorManager = std::make_unique<VulkanDescriptorManager>();
        TRY_deprecated(descriptorManager->create(_device->getLogicalDevice(), scheme, _framesInFlight, 1, errorMessage));

        VulkanDescriptorSets* descriptorSets = nullptr;
        TRY_deprecated(descriptorManager->allocate(descriptorSets, errorMessage));

        // Store descriptor manager and sets keyed by set index
        pass->getDescriptorManagers()[set] = std::move(descriptorManager);
        pass->getDescriptorSets()[set]     = descriptorSets;

        bindDescriptors(descriptorSets, scheme);
    }

    // Attach the layouts belonging to this pass
    auto& descriptorLayouts = pass->getPipelineDescriptor().descriptorLayouts;

    for (const auto& manager : pass->getDescriptorManagers() | std::views::values) {
        vk::DescriptorSetLayout layout = manager->getLayout();

        if (std::ranges::find(descriptorLayouts, layout) == descriptorLayouts.end()) {
            descriptorLayouts.push_back(layout);
        }
    }

    return true;
}

void VulkanRenderResources::bindDescriptors(
    const VulkanDescriptorSets* descriptorSets, const VulkanDescriptorScheme& scheme
) {
    for (const auto& descriptor : scheme) {
        auto itRes = _resources.find(descriptor.name);
        if (itRes == _resources.end()) continue;

        const auto& resource = itRes->second;
        if (!resource->image) continue;

        // Bind resource for each frame in flight
        descriptorSets->bindPerFrameResource(resource->image->getDescriptorInfo(descriptor.binding));
    }
}

void VulkanRenderResources::rebindDescriptors(VulkanRenderGraph& renderGraph) {
    for (const auto& pass : renderGraph.getPasses()) {

        for (const auto& [set, scheme] : pass->getShaderProgram()->getDescriptorSchemes()) {
            auto itSets = pass->getDescriptorSets().find(set);
            if (itSets == pass->getDescriptorSets().end()) continue;

            const VulkanDescriptorSets* descriptorSets = itSets->second;
            if (!descriptorSets) continue;

            bindDescriptors(descriptorSets, scheme);
        }

    }
}

void VulkanRenderResources::scheduleResourceTransitions() const {
    for (const auto& [resourceName, writerPasses] : _resourceWriters) {
        auto it = _resources.find(resourceName);
        if (it == _resources.end()) continue;

        VulkanRenderPassResource* resource = it->second.get();

        for (VulkanRenderPass* writerPass : writerPasses) {
            if (!writerPass) continue;
            writerPass->addTransition({resource, vk::ImageLayout::eShaderReadOnlyOptimal});
        }
    }
}
