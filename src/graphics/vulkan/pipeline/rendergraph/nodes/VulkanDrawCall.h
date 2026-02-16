#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgram.h"
#include "graphics/vulkan/resources/VulkanPushConstant.h"
#include "graphics/vulkan/resources/meshes/VulkanMesh.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

struct VulkanDrawCall {
    virtual ~VulkanDrawCall() = default;

    const VulkanMesh*         mesh  = nullptr;
    const VulkanRenderObject* owner = nullptr;

    std::vector<VulkanDescriptorSets*> descriptorSets{};

    std::optional<vk::Viewport> viewport{};
    std::optional<vk::Rect2D>   scissor{};

    [[nodiscard]] vk::Viewport resolveViewport(const vk::Extent2D extent) const {
        if (viewport) return *viewport;
        return vk::Viewport{
            0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f
        };
    }

    [[nodiscard]] vk::Rect2D resolveScissor(const vk::Extent2D extent) const {
        if (scissor) return *scissor;
        return {vk::Offset2D(0, 0), extent};
    }

    VulkanDrawCall& setMesh(const VulkanMesh* _mesh) noexcept { mesh = _mesh; return *this; }

    VulkanDrawCall& setOwner(const VulkanRenderObject* _owner) noexcept { owner = _owner; return *this; }

    VulkanDrawCall& setDescriptorSets(const std::vector<VulkanDescriptorSets*>& _descriptorSets) noexcept {
        descriptorSets = _descriptorSets;
        return *this;
    }

    VulkanDrawCall& setViewport(const vk::Viewport& _viewport) noexcept { viewport = _viewport; return *this; }

    VulkanDrawCall& setScissor(const vk::Rect2D _scissor) noexcept { scissor = _scissor; return *this; }

    VulkanDrawCall& addDescriptorSets(VulkanDescriptorSets* _descriptorSets) {
        descriptorSets.push_back(_descriptorSets);
        return *this;
    }
};

struct VulkanDrawCallWithPushConstants final : VulkanDrawCall {
    std::unordered_map<std::string, std::unique_ptr<IVulkanPushConstant>> pushConstantData;

    template <typename PushConstantType>
    VulkanDrawCallWithPushConstants& setPushConstant(const std::string& name, const PushConstantType* data) {
        pushConstantData[name] = std::make_unique<VulkanPushConstant<PushConstantType>>(data);
        return *this;
    }

    void pushConstants(
        const vk::CommandBuffer commandBuffer, const vk::PipelineLayout layout, const VulkanShaderProgram* program
    ) const {
        for (const auto& [name, range] : program->getPushConstants()) {
            if (auto it = pushConstantData.find(name); it != pushConstantData.end()) {
                it->second->push(commandBuffer, layout, range);
            }
        }
    }
};
