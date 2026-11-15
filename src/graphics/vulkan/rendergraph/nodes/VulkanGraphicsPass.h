#pragma once

#include "core/render/DrawFlags.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanPass.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipelineState.h"

class VulkanGraphicsRecordContext;

enum class VulkanGraphicsPassCullMode : std::uint8_t { None, Frustum };

struct VulkanGraphicsPassDescriptor {
    VulkanPassDescriptor base;

    std::function<Expected<void>(VulkanGraphicsRecordContext&)> record;

    VulkanGraphicsPipelineState pipelineState{};

    VulkanGraphicsPassCullMode cullMode = VulkanGraphicsPassCullMode::None;

    DrawFlags drawFlags{};

    bool countPrimitives = false;
};

class VulkanGraphicsPipeline;

class VulkanGraphicsPass {
public:

    explicit VulkanGraphicsPass(const VulkanGraphicsPassDescriptor& descriptor)
        : _pass(descriptor.base), _graphicsPassDescriptor(descriptor) {}

    ~VulkanGraphicsPass() = default;

    VulkanGraphicsPass(const VulkanGraphicsPass&)            = delete;
    VulkanGraphicsPass& operator=(const VulkanGraphicsPass&) = delete;

    VulkanGraphicsPass(VulkanGraphicsPass&&)            = delete;
    VulkanGraphicsPass& operator=(VulkanGraphicsPass&&) = delete;

    void destroy() const noexcept;

    // Getters

    [[nodiscard]]       VulkanPass& base()       noexcept { return _pass; }
    [[nodiscard]] const VulkanPass& base() const noexcept { return _pass; }

    [[nodiscard]] const VulkanGraphicsPassDescriptor& getGraphicsPassDescriptor() const noexcept {
        return _graphicsPassDescriptor;
    }

    [[nodiscard]] const VulkanGraphicsPipeline* getGraphicsPipeline() const noexcept { return _graphicsPipeline; }

    // Setters

    VulkanGraphicsPass& setGraphicsPipeline(const VulkanGraphicsPipeline* graphicsPipeline) noexcept {
        _graphicsPipeline = graphicsPipeline;
        return *this;
    }

private:
    VulkanPass _pass;

    VulkanGraphicsPassDescriptor _graphicsPassDescriptor;

    const VulkanGraphicsPipeline*  _graphicsPipeline = nullptr;
};
