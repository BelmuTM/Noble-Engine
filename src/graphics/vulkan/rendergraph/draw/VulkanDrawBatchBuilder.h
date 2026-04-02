#pragma once

#include "graphics/vulkan/resources/objects/VulkanRenderObjectManager.h"

#include "VulkanDrawBatch.h"
#include "VulkanInstanceHandle.h"

#include <unordered_map>
#include <vector>

class VulkanDrawBatchBuilder {
public:
    explicit VulkanDrawBatchBuilder(const VulkanRenderObjectManager& renderObjectManager)
        : _renderObjectManager(renderObjectManager) {}

    ~VulkanDrawBatchBuilder() = default;

    VulkanDrawBatchBuilder(const VulkanDrawBatchBuilder&)            = delete;
    VulkanDrawBatchBuilder& operator=(const VulkanDrawBatchBuilder&) = delete;

    VulkanDrawBatchBuilder(VulkanDrawBatchBuilder&&)            = delete;
    VulkanDrawBatchBuilder& operator=(VulkanDrawBatchBuilder&&) = delete;

    void build();

private:
    const VulkanRenderObjectManager& _renderObjectManager;

    std::unordered_map<VulkanDrawBatch, std::vector<VulkanInstanceHandle>> _batchMap{};
};
