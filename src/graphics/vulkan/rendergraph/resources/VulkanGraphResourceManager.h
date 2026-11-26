#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/core/VulkanDevice.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanPassBindings.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanPassDescriptor.h"
#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"

#include <memory>
#include <vector>

class VulkanGraphResourceManager {
public:
    using ResourcesMap = std::vector<std::unique_ptr<VulkanGraphResource>>;

    VulkanGraphResourceManager()  = default;
    ~VulkanGraphResourceManager() = default;

    VulkanGraphResourceManager(const VulkanGraphResourceManager&)            = delete;
    VulkanGraphResourceManager& operator=(const VulkanGraphResourceManager&) = delete;

    VulkanGraphResourceManager(VulkanGraphResourceManager&&)            = delete;
    VulkanGraphResourceManager& operator=(VulkanGraphResourceManager&&) = delete;

    [[nodiscard]] Expected<void> create(const VulkanDevice& device, std::uint32_t framesInFlight) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanGraphResourceId registerResource(const VulkanGraphResourceDescriptor& descriptor) {
        _registrations.push_back({.descriptor = descriptor});
        return static_cast<VulkanGraphResourceId>(_registrations.size() - 1);
    }

    [[nodiscard]] VulkanGraphResourceId registerExternalResource(
        const VulkanGraphResourceDescriptor& descriptor, const std::function<VulkanImage*()>& imageResolver
    ) {
        _registrations.push_back({.descriptor = descriptor, .imageResolver = imageResolver});
        return static_cast<VulkanGraphResourceId>(_registrations.size() - 1);
    }

    [[nodiscard]] Expected<void> recreate(vk::Extent2D extent);

    [[nodiscard]] Expected<void> allocateResources(vk::Extent2D extent);

    [[nodiscard]] Expected<const VulkanPassDescriptorBinding*> allocateDescriptors(
        const VulkanPassDescriptor& passDescriptor
    );

    [[nodiscard]] const ResourcesMap& getResources() const noexcept { return _resources; }

    [[nodiscard]] VulkanGraphResource* getResource(const VulkanGraphResourceId id) noexcept {
        return id < _resources.size() ? _resources[id].get() : nullptr;
    }

    [[nodiscard]] const VulkanGraphResource* getResource(const VulkanGraphResourceId id) const noexcept {
        return id < _resources.size() ? _resources[id].get() : nullptr;
    }

private:
    struct Registration {
        VulkanGraphResourceDescriptor descriptor;
        std::function<VulkanImage*()> imageResolver{};
    };

    [[nodiscard]] static Expected<void> createResourceImage(
        VulkanImage&        resourceImage,
        vk::Format          format,
        vk::Extent2D        extent,
        const VulkanDevice* device
    );

    void bindDescriptors(const VulkanPassDescriptorBinding* binding);

    void rebindDescriptors();

    const VulkanDevice* _device = nullptr;

    std::uint32_t _framesInFlight = 0;

    std::vector<Registration> _registrations{};

    // Global resources cache
    ResourcesMap _resources{};

    // Instances of the images that resources hold pointers of
    std::vector<std::unique_ptr<VulkanImage>> _resourceImages{};

    // Per-pass descriptor bindings
    std::vector<std::unique_ptr<VulkanPassDescriptorBinding>> _passBindings{};
};
