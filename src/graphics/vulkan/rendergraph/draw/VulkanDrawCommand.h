#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"
#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgram.h"

#include "graphics/vulkan/resources/meshes/VulkanMesh.h"

struct VulkanPipelineLayoutDescriptor;

class VulkanDrawCommand {
public:
    VulkanDrawCommand()  = default;
    ~VulkanDrawCommand() = default;

    VulkanDrawCommand(const VulkanDrawCommand&)            = delete;
    VulkanDrawCommand& operator=(const VulkanDrawCommand&) = delete;

    VulkanDrawCommand(VulkanDrawCommand&&)            noexcept = default;
    VulkanDrawCommand& operator=(VulkanDrawCommand&&) noexcept = default;

    void record(
        vk::CommandBuffer  commandBuffer,
        vk::Extent2D       extent,
        vk::PipelineLayout pipelineLayout,
        std::uint32_t      instanceCount = 1,
        std::uint32_t      firstInstance = 0
    ) const;

    void pushConstants(vk::CommandBuffer commandBuffer, vk::PipelineLayout pipelineLayout) const noexcept;

    [[nodiscard]] vk::Viewport resolveViewport(vk::Extent2D extent) const;
    [[nodiscard]] vk::Rect2D resolveScissor(vk::Extent2D extent) const;

    [[nodiscard]] const VulkanMesh* getMesh() const noexcept { return _mesh; }

    VulkanDrawCommand& setMesh(const VulkanMesh* mesh) noexcept {
        _mesh = mesh;
        return *this;
    }

    VulkanDrawCommand& setViewport(const vk::Viewport& viewport) noexcept { _viewport = viewport; return *this; }
    VulkanDrawCommand& setScissor(const vk::Rect2D scissor) noexcept { _scissor = scissor; return *this; }

    VulkanDrawCommand& bindPipelineLayoutDescriptor(const VulkanPipelineLayoutDescriptor* descriptor) noexcept {
        _pipelineLayoutDescriptor = descriptor;
        return *this;
    }

    template<typename PushConstantType>
    VulkanDrawCommand& setPushConstant(const std::string& name, const PushConstantType* data) noexcept {
        const auto cachedRange = _pipelineLayoutDescriptor->pushConstantRanges.find(name);

        // TODO: Implement ASSERT() macro abstraction that calls Engine::fatalExit(message)
        assert(cachedRange != _pipelineLayoutDescriptor->pushConstantRanges.end());

        _pushConstants[name] = VulkanPushConstant(data, cachedRange->second);

        return *this;
    }

private:
    const VulkanMesh* _mesh = nullptr;

    // WARNING: Mutable, should store copy instead of pointer if layout ever changes during runtime
    // (e.g.: interface-breaking hot reloads).
    const VulkanPipelineLayoutDescriptor* _pipelineLayoutDescriptor = nullptr;

    std::unordered_map<std::string, VulkanPushConstant> _pushConstants{};

    std::optional<vk::Viewport> _viewport{};
    std::optional<vk::Rect2D>   _scissor{};
};
