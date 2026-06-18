#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"
#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgram.h"

#include "graphics/vulkan/resources/meshes/VulkanRenderMesh.h"

#include "graphics/vulkan/rendergraph/draw/VulkanInstanceHandle.h"

struct VulkanPipelineLayoutDescriptor;

class VulkanDrawCall {
public:
    VulkanDrawCall()  = default;
    ~VulkanDrawCall() = default;

    VulkanDrawCall(const VulkanDrawCall&)            = delete;
    VulkanDrawCall& operator=(const VulkanDrawCall&) = delete;

    VulkanDrawCall(VulkanDrawCall&&)            noexcept = default;
    VulkanDrawCall& operator=(VulkanDrawCall&&) noexcept = default;

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

    [[nodiscard]] const std::string& getName() const noexcept { return _name; }
    [[nodiscard]] const VulkanRenderMesh& getRenderMesh() const noexcept { return _renderMesh; }
    [[nodiscard]] const VulkanInstanceHandle& getInstanceHandle() const noexcept { return _instanceHandle; }
    [[nodiscard]] const glm::mat4* getModelMatrix() const noexcept { return _modelMatrix; }
    [[nodiscard]] const std::vector<const VulkanDescriptorSets*>& getDescriptorSets() const noexcept {
        return _descriptorSets;
    }

    VulkanDrawCall& setName(const std::string& name) noexcept { _name = name; return *this; }

    VulkanDrawCall& setRenderMesh(const VulkanRenderMesh& renderMesh) noexcept {
        _renderMesh = renderMesh;
        return *this;
    }

    VulkanDrawCall& setInstanceHandle(const VulkanInstanceHandle& instanceHandle) noexcept {
        _instanceHandle = instanceHandle;
        return *this;
    }

    VulkanDrawCall& setModelMatrix(const glm::mat4& modelMatrix) noexcept { _modelMatrix = &modelMatrix; return *this; }

    VulkanDrawCall& setViewport(const vk::Viewport& viewport) noexcept { _viewport = viewport; return *this; }
    VulkanDrawCall& setScissor(const vk::Rect2D scissor) noexcept { _scissor = scissor; return *this; }

    VulkanDrawCall& setDescriptorSets(const std::vector<const VulkanDescriptorSets*>& descriptorSets) noexcept {
        _descriptorSets = descriptorSets;
        return *this;
    }

    VulkanDrawCall& addDescriptorSets(const VulkanDescriptorSets* descriptorSets) {
        _descriptorSets.push_back(descriptorSets);
        return *this;
    }

    VulkanDrawCall& bindPipelineLayoutDescriptor(const VulkanPipelineLayoutDescriptor* descriptor) noexcept {
        _pipelineLayoutDescriptor = descriptor;
        return *this;
    }

    template<typename PushConstantType>
    VulkanDrawCall& setPushConstant(const std::string& name, const PushConstantType* data) noexcept {
        const auto cachedRange = _pipelineLayoutDescriptor->pushConstantRanges.find(name);

        // TODO: Implement ASSERT() macro abstraction that calls Engine::fatalExit(message)
        assert(cachedRange != _pipelineLayoutDescriptor->pushConstantRanges.end());

        _pushConstants[name] = VulkanPushConstant(data, cachedRange->second);

        return *this;
    }

private:
    std::string _name = "Undefined_Drawcall";

    VulkanRenderMesh     _renderMesh{};
    VulkanInstanceHandle _instanceHandle{};

    const glm::mat4* _modelMatrix = nullptr;

    std::vector<const VulkanDescriptorSets*> _descriptorSets{};

    // WARNING: Mutable, should store copy instead of pointer if layout ever changes during runtime
    // (e.g.: interface-breaking hot reloads).
    const VulkanPipelineLayoutDescriptor* _pipelineLayoutDescriptor = nullptr;

    std::unordered_map<std::string, VulkanPushConstant> _pushConstants{};

    std::optional<vk::Viewport> _viewport{};
    std::optional<vk::Rect2D>   _scissor{};
};
