#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgram.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

class VulkanDrawCall {
public:
    VulkanDrawCall()  = default;
    ~VulkanDrawCall() = default;

    VulkanDrawCall(const VulkanDrawCall&)            = delete;
    VulkanDrawCall& operator=(const VulkanDrawCall&) = delete;

    VulkanDrawCall(VulkanDrawCall&&)            noexcept = default;
    VulkanDrawCall& operator=(VulkanDrawCall&&) noexcept = default;

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

    [[nodiscard]] const std::string& getName() const noexcept { return _name; }
    [[nodiscard]] const VulkanMesh* getMesh() const noexcept { return _mesh; }
    [[nodiscard]] const glm::mat4* getModelMatrix() const noexcept { return _modelMatrix; }
    [[nodiscard]] const std::vector<const VulkanDescriptorSets*>& getDescriptorSets() const noexcept {
        return _descriptorSets;
    }

    VulkanDrawCall& setName(const std::string& name) noexcept { _name = name; return *this; }
    VulkanDrawCall& setMesh(const VulkanMesh* mesh) noexcept { _mesh = mesh; return *this; }
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
    template<typename PushConstantType>
    VulkanDrawCall& setPushConstant(const std::string& name, const PushConstantType* data) noexcept {
        _pushConstants[name] = std::make_unique<VulkanPushConstant<PushConstantType>>(data);
        return *this;
    }

private:
    std::string _name = "Undefined_Drawcall";

    const VulkanMesh* _mesh = nullptr;

    const glm::mat4* _modelMatrix = nullptr;

    std::vector<const VulkanDescriptorSets*> _descriptorSets{};

    std::unordered_map<std::string, std::unique_ptr<IVulkanPushConstant>> _pushConstants{};

    std::optional<vk::Viewport> _viewport{};
    std::optional<vk::Rect2D>   _scissor{};
};
