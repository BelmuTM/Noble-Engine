#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderPassResource.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanRenderGraph;

class VulkanRenderResourceManager {
public:
    using ResourcesMap         = std::unordered_map<std::string, std::unique_ptr<VulkanRenderPassResource>>;
    using ResourceAccessorsMap = std::unordered_map<std::string, std::vector<VulkanRenderPass*>>;
    using ResourceImagesMap    = std::unordered_map<std::string, std::unique_ptr<VulkanImage>>;

    VulkanRenderResourceManager()  = default;
    ~VulkanRenderResourceManager() = default;

    VulkanRenderResourceManager(const VulkanRenderResourceManager&)            = delete;
    VulkanRenderResourceManager& operator=(const VulkanRenderResourceManager&) = delete;

    VulkanRenderResourceManager(VulkanRenderResourceManager&&)            = delete;
    VulkanRenderResourceManager& operator=(VulkanRenderResourceManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice&         device,
        const VulkanSwapchain&      swapchain,
        const VulkanCommandManager& commandManager,
        std::uint32_t               framesInFlight
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> recreate(VulkanRenderGraph& renderGraph);

    [[nodiscard]] Expected<void> createResource(const VulkanRenderPassResourceDescriptor& descriptor);

    [[nodiscard]] Expected<void> allocateDescriptors(VulkanRenderPass* pass);

    [[nodiscard]] const ResourcesMap& getResources() const noexcept { return _resources; }

    [[nodiscard]]       ResourceAccessorsMap& getResourceReaders()       noexcept { return _resourceReaders; }
    [[nodiscard]] const ResourceAccessorsMap& getResourceReaders() const noexcept { return _resourceReaders; }

    [[nodiscard]]       ResourceAccessorsMap& getResourceWriters()       noexcept { return _resourceWriters; }
    [[nodiscard]] const ResourceAccessorsMap& getResourceWriters() const noexcept { return _resourceWriters; }

    [[nodiscard]] VulkanRenderPassResource* getResource(const std::string& name) noexcept {
        const auto cachedRenderResource = _resources.find(name);
        return cachedRenderResource != _resources.end() ? cachedRenderResource->second.get() : nullptr;
    }

    void addResource(const VulkanRenderPassResource& resource) {
        if (!_resources.contains(resource.descriptor.name)) {
            _resources[resource.descriptor.name] = std::make_unique<VulkanRenderPassResource>(resource);
        }
    }

    void addResourceReader(const std::string& name, VulkanRenderPass* pass) {
        _resourceReaders[name].push_back(pass);
    }

    void addResourceWriter(const std::string& name, VulkanRenderPass* pass) {
        _resourceWriters[name].push_back(pass);
    }

private:
    [[nodiscard]] static Expected<void> createResourceImage(
        VulkanImage&                resourceImage,
        vk::Format                  format,
        vk::Extent2D                extent,
        const VulkanDevice*         device,
        const VulkanCommandManager* commandManager
    );

    void bindDescriptors(
        const VulkanDescriptorSets* descriptorSets, const std::vector<VulkanRenderPassAttachmentDescriptor>& reads
    );

    void rebindDescriptors(VulkanRenderGraph& renderGraph);

    const VulkanDevice*         _device         = nullptr;
    const VulkanSwapchain*      _swapchain      = nullptr;
    const VulkanCommandManager* _commandManager = nullptr;

    std::uint32_t _framesInFlight = 0;

    // Global resources and resource accessors cache
    ResourcesMap         _resources{};
    ResourceAccessorsMap _resourceReaders{};
    ResourceAccessorsMap _resourceWriters{};

    // Instances of the images that resources hold pointers of
    ResourceImagesMap _resourceImages{};
};
