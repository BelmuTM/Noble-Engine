#pragma once

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPassResource.h"

#include <memory>
#include <unordered_map>
#include <vector>

class VulkanRenderGraph;

class VulkanRenderResources {
public:
    // WARNING: case-sensitive keyword reserved for the depth buffer resource
    static constexpr std::string DEPTH_BUFFER_RESOURCE_NAME = "depthBuffer";
    static constexpr auto        DEPTH_BUFFER_FORMAT        = vk::Format::eD32Sfloat;

    using ResourcesMap         = std::unordered_map<std::string, std::unique_ptr<VulkanRenderPassResource>>;
    using ResourceAccessorsMap = std::unordered_map<std::string, std::vector<VulkanRenderPass*>>;

    VulkanRenderResources()  = default;
    ~VulkanRenderResources() = default;

    VulkanRenderResources(const VulkanRenderResources&)            = delete;
    VulkanRenderResources& operator=(const VulkanRenderResources&) = delete;

    VulkanRenderResources(VulkanRenderResources&&)            = delete;
    VulkanRenderResources& operator=(VulkanRenderResources&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&         device,
        const VulkanSwapchain&      swapchain,
        const VulkanCommandManager& commandManager,
        uint32_t                    framesInFlight,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool recreate(VulkanRenderGraph& renderGraph, std::string& errorMessage);

    [[nodiscard]] bool createDepthBuffer(std::string& errorMessage);

    [[nodiscard]] bool createColorBuffers(
        VulkanRenderPass* pass, const VulkanFrameResources& frameResources, std::string& errorMessage
    );

    [[nodiscard]] bool allocateDescriptors(VulkanRenderPass* pass, std::string& errorMessage);

    [[nodiscard]] const ResourcesMap& getResources() const noexcept {
        return _resources;
    }

    [[nodiscard]]       ResourceAccessorsMap& getResourceReaders()       noexcept { return _resourceReaders; }
    [[nodiscard]] const ResourceAccessorsMap& getResourceReaders() const noexcept { return _resourceReaders; }

    [[nodiscard]]       ResourceAccessorsMap& getResourceWriters()       noexcept { return _resourceWriters; }
    [[nodiscard]] const ResourceAccessorsMap& getResourceWriters() const noexcept { return _resourceWriters; }

    [[nodiscard]] VulkanRenderPassAttachment* getDepthBufferAttachment() const noexcept {
        return _depthBufferAttachment.get();
    }

    void addResource(const VulkanRenderPassResource& resource) {
        if (!_resources.contains(resource.name)) {
            _resources[resource.name] = std::make_unique<VulkanRenderPassResource>(resource);
        }
    }

    void addResourceReader(const std::string& name, VulkanRenderPass* pass) {
        _resourceReaders[name].push_back(pass);
    }

    void addResourceWriter(const std::string& name, VulkanRenderPass* pass) {
        _resourceWriters[name].push_back(pass);
    }

private:
    [[nodiscard]] bool createColorBufferImage(
        VulkanImage& colorBuffer, vk::Format format, vk::Extent2D extent, std::string& errorMessage
    ) const;

    [[nodiscard]] bool createDepthBufferImage(
        VulkanImage& depthBuffer, vk::Extent2D extent, std::string& errorMessage
    ) const;

    void bindDescriptors(const VulkanDescriptorSets* descriptorSets, const VulkanDescriptorScheme& scheme);

    void rebindDescriptors(VulkanRenderGraph& renderGraph);

    const VulkanDevice*         _device         = nullptr;
    const VulkanSwapchain*      _swapchain      = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    uint32_t _framesInFlight = 0;

    // Global resources and resource accessors cache
    ResourcesMap         _resources{};
    ResourceAccessorsMap _resourceReaders{};
    ResourceAccessorsMap _resourceWriters{};

    // Depth buffer
    std::unique_ptr<VulkanImage>                _depthBuffer{};
    std::unique_ptr<VulkanRenderPassAttachment> _depthBufferAttachment{};

    // Color buffers
    std::vector<std::unique_ptr<VulkanImage>> _colorBuffers{};
};
