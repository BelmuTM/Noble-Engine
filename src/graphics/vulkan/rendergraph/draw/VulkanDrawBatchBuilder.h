#pragma once

#include "graphics/vulkan/rendergraph/draw/VulkanDrawCommand.h"
#include "graphics/vulkan/rendergraph/draw/VulkanRenderItem.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanGraphicsPass.h"

#include "graphics/vulkan/rendergraph/resources/VulkanRenderResourceManager.h"

#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"
#include "graphics/vulkan/resources/ssbo/VulkanStorageBufferManager.h"

#include <span>
#include <vector>

class VulkanDrawBatchBuilder {
public:
    static constexpr std::uint32_t MAX_INSTANCES = 5'000'000;

    VulkanDrawBatchBuilder()  = default;
    ~VulkanDrawBatchBuilder() = default;

    VulkanDrawBatchBuilder(const VulkanDrawBatchBuilder&)            = delete;
    VulkanDrawBatchBuilder& operator=(const VulkanDrawBatchBuilder&) = delete;

    VulkanDrawBatchBuilder(VulkanDrawBatchBuilder&&)            = delete;
    VulkanDrawBatchBuilder& operator=(VulkanDrawBatchBuilder&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice&         device,
        VulkanStorageBufferManager& storageBufferManager,
        const VulkanStorageBuffer*  objectBuffer,
        std::uint32_t               framesInFlight
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> build(
        std::span<const std::unique_ptr<VulkanGraphicsPass>> passes,
        std::span<const VulkanRenderItem>                    allItems,
        std::span<const VulkanRenderItem>                    visibleItems,
        std::uint32_t                                        frameIndex
   );

    [[nodiscard]] const VulkanDrawCommandsVector& getDrawBatch(const VulkanGraphicsPass& pass) const {
        return _drawBatches[pass.base().getIndex()];
    }

    [[nodiscard]] static VulkanDescriptorScheme getDescriptorScheme() noexcept {
        return {
            {.binding = 0, .type = vk::DescriptorType::eStorageBuffer, .stageFlags = vk::ShaderStageFlagBits::eVertex},
            {.binding = 1, .type = vk::DescriptorType::eStorageBuffer, .stageFlags = vk::ShaderStageFlagBits::eVertex}
        };
    }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _descriptors; }

private:
    struct Entry {
        std::uint64_t key = 0;
        std::uint32_t objectIndex = 0;

        VulkanRenderMesh renderMesh{};
    };

    std::vector<VulkanDrawCommandsVector> _drawBatches{};
    std::vector<Entry>                    _entries{};
    std::vector<std::uint32_t>            _indirection{};

    VulkanDescriptorManager _descriptorManager{};
    VulkanDescriptorSets*   _descriptors = nullptr;

    VulkanStorageBuffer* _indirectionBuffer = nullptr;
};
