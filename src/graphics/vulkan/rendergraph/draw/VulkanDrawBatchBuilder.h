#pragma once

#include "VulkanDrawBatch.h"
#include "VulkanInstanceHandle.h"

#include "VulkanRenderItem.h"

#include <unordered_map>
#include <vector>

struct VulkanBuiltDrawBatch {
    VulkanDrawBatch batch{};
    std::uint32_t   firstInstance = 0;
    std::uint32_t   instanceCount = 0;
};

class VulkanDrawBatchBuilder {
public:
    VulkanDrawBatchBuilder()  = default;
    ~VulkanDrawBatchBuilder() = default;

    VulkanDrawBatchBuilder(const VulkanDrawBatchBuilder&)            = delete;
    VulkanDrawBatchBuilder& operator=(const VulkanDrawBatchBuilder&) = delete;

    VulkanDrawBatchBuilder(VulkanDrawBatchBuilder&&)            = delete;
    VulkanDrawBatchBuilder& operator=(VulkanDrawBatchBuilder&&) = delete;

    void build(const std::vector<VulkanRenderItem>& renderItems, std::uint32_t indirectionOffset);

    [[nodiscard]] std::vector<VulkanBuiltDrawBatch>& getBuiltDrawBatches() noexcept { return _builtDrawBatches; }

    [[nodiscard]] const std::vector<std::uint32_t>& getIndirectionData() const noexcept { return _indirectionData; }

private:
    std::unordered_map<VulkanDrawBatch, std::vector<VulkanInstanceHandle>> _drawBatchMap{};

    std::vector<VulkanBuiltDrawBatch> _builtDrawBatches{};

    std::vector<std::uint32_t> _indirectionData{};
};
