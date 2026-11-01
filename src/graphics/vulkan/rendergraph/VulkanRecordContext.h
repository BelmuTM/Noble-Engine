#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/rendergraph/VulkanFrameContext.h"
#include "graphics/vulkan/rendergraph/nodes/VulkanPassBindings.h"

#include <string>
#include <type_traits>

class VulkanMaterial;

class VulkanRecordContext {
public:
    VulkanRecordContext(
        const vk::CommandBuffer   commandBuffer,
        const VulkanFrameContext& frame,
        const VulkanPassBindings& pass
    )
        :
        _commandBuffer(commandBuffer),
        _frame(frame),
        _pass(pass)
    {}

    VulkanRecordContext(const VulkanRecordContext&)            = delete;
    VulkanRecordContext& operator=(const VulkanRecordContext&) = delete;

    VulkanRecordContext(VulkanRecordContext&&)            = delete;
    VulkanRecordContext& operator=(VulkanRecordContext&&) = delete;

    // Binding

    void bindPipeline() const;
    void bindGlobalDescriptors() const;
    void bindPassDescriptors() const;
    void bindMaterial(const VulkanMaterial* material);
    void bindGeometryBuffers() const;

    // Dynamic state

    void setViewport() const;
    void setScissor() const;

    template<typename PushConstantType>
    void pushConstant(const std::string& name, const PushConstantType& value) {

        static_assert(std::is_trivially_copyable_v<PushConstantType>);

        pushConstantRaw(name, &value, static_cast<std::uint32_t>(sizeof(PushConstantType)));
    }

    [[nodiscard]] vk::CommandBuffer commandBuffer() const noexcept { return _commandBuffer; }
    [[nodiscard]] vk::Extent2D extent() const noexcept { return _frame.extent; }
    [[nodiscard]] std::uint32_t frameIndex() const noexcept { return _frame.frameIndex; }

private:
    void pushConstantRaw(const std::string& name, const void* data, std::uint32_t typeSize) const;

    vk::CommandBuffer _commandBuffer;

    const VulkanFrameContext& _frame;
    const VulkanPassBindings& _pass;

    const VulkanMaterial* _lastBoundMaterial = nullptr;
};
