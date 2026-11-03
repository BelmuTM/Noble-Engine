#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/rendergraph/VulkanFrameContext.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanPassBindings.h"

#include "graphics/vulkan/rendergraph/draw/VulkanDrawCommand.h"

#include "VulkanRecordContext.h"

class VulkanMaterial;

class VulkanGraphicsRecordContext final : public VulkanRecordContext {
public:
    VulkanGraphicsRecordContext(
        const vk::CommandBuffer         commandBuffer,
        const VulkanFrameContext&       frame,
        const VulkanPassBindings&       pass,
        const VulkanDrawCommandsVector& drawBatches
    ) noexcept
        :
        VulkanRecordContext(commandBuffer, frame, pass), _drawBatches(drawBatches)
    {}

    VulkanGraphicsRecordContext(const VulkanGraphicsRecordContext&)            = delete;
    VulkanGraphicsRecordContext& operator=(const VulkanGraphicsRecordContext&) = delete;

    VulkanGraphicsRecordContext(VulkanGraphicsRecordContext&&)            = delete;
    VulkanGraphicsRecordContext& operator=(VulkanGraphicsRecordContext&&) = delete;

    // Binding

    void bindMaterial(const VulkanMaterial* material);
    void bindGeometryBuffers() const;

    // Drawing

    void draw(
        std::uint32_t vertexCount,
        std::uint32_t instanceCount = 1,
        std::uint32_t firstVertex   = 0,
        std::uint32_t firstInstance = 0
    ) const;

    void drawBatch(const VulkanDrawCommand& batch);
    void drawBatches();

    // Dynamic state

    void setViewport() const;
    void setScissor() const;

private:
    const VulkanDrawCommandsVector& _drawBatches;

    const VulkanMaterial* _lastBoundMaterial = nullptr;
};
