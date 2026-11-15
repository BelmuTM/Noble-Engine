#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanPass.h"
#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class VulkanRenderGraph;

class VulkanGraphResourceManager {
public:
    using ResourcesMap         = std::unordered_map<std::string, std::unique_ptr<VulkanGraphResource>>;
    using ResourceAccessorsMap = std::unordered_map<std::string, std::vector<VulkanPass*>>;
    using ResourceImagesMap    = std::unordered_map<std::string, std::unique_ptr<VulkanImage>>;

    VulkanGraphResourceManager()  = default;
    ~VulkanGraphResourceManager() = default;

    VulkanGraphResourceManager(const VulkanGraphResourceManager&)            = delete;
    VulkanGraphResourceManager& operator=(const VulkanGraphResourceManager&) = delete;

    VulkanGraphResourceManager(VulkanGraphResourceManager&&)            = delete;
    VulkanGraphResourceManager& operator=(VulkanGraphResourceManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice&    device,
        const VulkanSwapchain& swapchain,
        std::uint32_t          framesInFlight
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> recreate(VulkanRenderGraph& renderGraph);

    [[nodiscard]] Expected<void> createResource(const VulkanGraphResourceDescriptor& descriptor);

    [[nodiscard]] Expected<void> allocateDescriptors(VulkanPass* pass);

    [[nodiscard]] const ResourcesMap& getResources() const noexcept { return _resources; }

    [[nodiscard]] VulkanGraphResource* getResource(const std::string& name) noexcept {
        const auto cachedRenderResource = _resources.find(name);
        return cachedRenderResource != _resources.end() ? cachedRenderResource->second.get() : nullptr;
    }

    [[nodiscard]] const VulkanGraphResource* getResource(const std::string& name) const noexcept {
        const auto cachedRenderResource = _resources.find(name);
        return cachedRenderResource != _resources.end() ? cachedRenderResource->second.get() : nullptr;
    }

    void addResource(const VulkanGraphResource& resource) {
        if (!_resources.contains(resource.descriptor.name)) {
            _resources[resource.descriptor.name] = std::make_unique<VulkanGraphResource>(resource);
        }
    }

private:
    [[nodiscard]] static Expected<void> createResourceImage(
        VulkanImage&        resourceImage,
        vk::Format          format,
        vk::Extent2D        extent,
        const VulkanDevice* device
    );

    void bindDescriptors(
        const VulkanDescriptorSets* descriptorSets, const std::vector<std::string>& sampledResourcesNames
    );

    void rebindDescriptors(VulkanRenderGraph& renderGraph);

    const VulkanDevice*    _device    = nullptr;
    const VulkanSwapchain* _swapchain = nullptr;

    std::uint32_t _framesInFlight = 0;

    // Global resources cache
    ResourcesMap _resources{};

    // Instances of the images that resources hold pointers of
    ResourceImagesMap _resourceImages{};
};
