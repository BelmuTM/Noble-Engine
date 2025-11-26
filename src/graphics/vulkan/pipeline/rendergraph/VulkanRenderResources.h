#pragma once
#ifndef NOBLEENGINE_VULKANRENDERRESOURCES_H
#define NOBLEENGINE_VULKANRENDERRESOURCES_H

#include "nodes/VulkanRenderPassResource.h"

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"

#include "nodes/VulkanRenderPass.h"

#include <memory>
#include <unordered_map>
#include <vector>

class VulkanRenderPass;

class VulkanRenderResources {
public:
    // WARNING: DEPTH_BUFFER_NAME is the case-sensitive keyword reserved to the depth buffer resource
    static constexpr std::string DEPTH_BUFFER_NAME = "depthTexture";

    using ResourcesMap         = std::unordered_map<std::string, std::unique_ptr<VulkanRenderPassResource>>;
    using ResourceAccessorsMap = std::unordered_map<std::string, std::vector<VulkanRenderPass*>>;

    VulkanRenderResources()  = default;
    ~VulkanRenderResources() = default;

    VulkanRenderResources(const VulkanRenderResources&)            = delete;
    VulkanRenderResources& operator=(const VulkanRenderResources&) = delete;

    VulkanRenderResources(VulkanRenderResources&&)            = delete;
    VulkanRenderResources& operator=(VulkanRenderResources&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&       device,
        const VulkanSwapchain&    swapchain,
        const VulkanImageManager& imageManager,
        uint32_t                  framesInFlight,
        std::string&              errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool recreate(std::string& errorMessage);

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

    [[nodiscard]] bool createColorAttachments(
        VulkanRenderPass*         pass,
        const VulkanImageManager& imageManager,
        VulkanFrameResources&     frameResources,
        std::string&              errorMessage
    );

    [[nodiscard]] bool allocateDescriptors(VulkanRenderPass* pass, std::string& errorMessage);

    std::vector<vk::DescriptorSet> buildDescriptorSets(uint32_t frameIndex) const;

private:
    const VulkanDevice*       _device       = nullptr;
    const VulkanSwapchain*    _swapchain    = nullptr;
    const VulkanImageManager* _imageManager = nullptr;

    uint32_t _framesInFlight = 0;

    // Global resources and resource accessors cache
    ResourcesMap         _resources{};
    ResourceAccessorsMap _resourceReaders{};
    ResourceAccessorsMap _resourceWriters{};

    std::vector<std::unique_ptr<VulkanDescriptorManager>> _descriptorManagers{};
    std::vector<VulkanDescriptorSets*>                    _descriptorSetGroups{};

    // Descriptor bindings cache
    struct BindingEntry {
        uint32_t    binding;
        std::string resourceName;
    };

    std::vector<std::vector<BindingEntry>> _descriptorBindingsPerManager{};

    // Depth buffer
    std::unique_ptr<VulkanImage>                _depthBuffer{};
    std::unique_ptr<VulkanRenderPassAttachment> _depthBufferAttachment{};
};

#endif // NOBLEENGINE_VULKANRENDERRESOURCES_H
