#pragma once

#include "VulkanDrawCall.h"

#include "VulkanDrawBatch.h"
#include "VulkanInstanceHandle.h"

#include <unordered_map>
#include <vector>

class VulkanDrawBatchBuilder {
public:
    struct BuiltDrawBatch {
        VulkanDrawCall* drawCall;
        std::uint32_t   firstInstance;
        std::uint32_t   instanceCount;
    };

    using BatchValue         = std::pair<VulkanDrawCall*, std::vector<VulkanInstanceHandle>>;
    using VulkanDrawBatchMap = std::unordered_map<VulkanDrawBatch, BatchValue>;

    VulkanDrawBatchBuilder()  = default;
    ~VulkanDrawBatchBuilder() = default;

    VulkanDrawBatchBuilder(const VulkanDrawBatchBuilder&)            = delete;
    VulkanDrawBatchBuilder& operator=(const VulkanDrawBatchBuilder&) = delete;

    VulkanDrawBatchBuilder(VulkanDrawBatchBuilder&&)            = delete;
    VulkanDrawBatchBuilder& operator=(VulkanDrawBatchBuilder&&) = delete;

    void build(const std::vector<VulkanDrawCall*>& drawCalls, std::uint32_t indirectionOffset);

    [[nodiscard]] const VulkanDrawBatchMap& getDrawBatchMap() const noexcept { return _drawBatchMap; }

    [[nodiscard]] std::vector<BuiltDrawBatch>& getBuiltDrawBatches() noexcept { return _builtDrawBatches; }

    [[nodiscard]] const std::vector<std::uint32_t>& getIndirectionData() const noexcept { return _indirectionData; }

private:
    VulkanDrawBatchMap _drawBatchMap{};

    std::vector<BuiltDrawBatch> _builtDrawBatches{};

    std::vector<std::uint32_t> _indirectionData{};
};
