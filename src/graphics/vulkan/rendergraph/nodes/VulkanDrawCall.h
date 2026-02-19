#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgram.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

class VulkanDrawCall {
public:
    VulkanDrawCall()  = default;
    ~VulkanDrawCall() = default;

    void record(
        vk::CommandBuffer          commandBuffer,
        vk::Extent2D               extent,
        vk::PipelineLayout         pipelineLayout,
        const VulkanShaderProgram* shaderProgram
    ) const;

    void pushConstants(
        vk::CommandBuffer          commandBuffer,
        vk::PipelineLayout         pipelineLayout,
        const VulkanShaderProgram* shaderProgram
    ) const noexcept;

    [[nodiscard]] vk::Viewport resolveViewport(vk::Extent2D extent) const;
    [[nodiscard]] vk::Rect2D resolveScissor(vk::Extent2D extent) const;

    [[nodiscard]] const VulkanMesh* getMesh() const noexcept { return _mesh; }
    [[nodiscard]] const VulkanRenderObject* getOwner() const noexcept { return _owner; }
    [[nodiscard]] const std::vector<VulkanDescriptorSets*>& getDescriptorSets() const noexcept {
        return _descriptorSets;
    }

    VulkanDrawCall& setMesh(const VulkanMesh* mesh) noexcept { _mesh = mesh; return *this; }
    VulkanDrawCall& setOwner(const VulkanRenderObject* owner) noexcept { _owner = owner; return *this; }
    VulkanDrawCall& setViewport(const vk::Viewport& viewport) noexcept { _viewport = viewport; return *this; }
    VulkanDrawCall& setScissor(const vk::Rect2D scissor) noexcept { _scissor = scissor; return *this; }
    VulkanDrawCall& setDescriptorSets(const std::vector<VulkanDescriptorSets*>& descriptorSets) noexcept {
        _descriptorSets = descriptorSets;
        return *this;
    }
    VulkanDrawCall& addDescriptorSets(VulkanDescriptorSets* descriptorSets) {
        _descriptorSets.push_back(descriptorSets);
        return *this;
    }
    template <typename PushConstantType>
    VulkanDrawCall& setPushConstant(const std::string& name, const PushConstantType* data) noexcept {
        _pushConstants[name] = std::make_unique<VulkanPushConstant<PushConstantType>>(data);
        return *this;
    }

private:
    const VulkanMesh*         _mesh  = nullptr;
    const VulkanRenderObject* _owner = nullptr;

    std::vector<VulkanDescriptorSets*> _descriptorSets{};

    std::unordered_map<std::string, std::unique_ptr<IVulkanPushConstant>> _pushConstants;

    std::optional<vk::Viewport> _viewport{};
    std::optional<vk::Rect2D>   _scissor{};
};
