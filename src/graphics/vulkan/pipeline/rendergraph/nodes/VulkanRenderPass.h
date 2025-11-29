#pragma once
#ifndef NOBLEENGINE_VULKANRENDERPASS_H
#define NOBLEENGINE_VULKANRENDERPASS_H

#include "VulkanDrawCall.h"
#include "VulkanRenderPassAttachment.h"

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

#include <memory>
#include <ranges>

struct VulkanPassTransition {
    VulkanRenderPassResource* resource     = nullptr;
    vk::ImageLayout           targetLayout = vk::ImageLayout::eUndefined;

    bool operator==(const VulkanPassTransition& other) const {
        return resource == other.resource && targetLayout == other.targetLayout;
    }
};

class VulkanRenderPass {
public:
    using DescriptorSetsMap     = std::unordered_map<uint32_t, VulkanDescriptorSets*>;
    using DescriptorManagersMap = std::unordered_map<uint32_t, std::unique_ptr<VulkanDescriptorManager>>;

    VulkanRenderPass()  = default;
    ~VulkanRenderPass() = default;

    VulkanRenderPass(const VulkanRenderPass&)            = delete;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    VulkanRenderPass(VulkanRenderPass&&)            = delete;
    VulkanRenderPass& operator=(VulkanRenderPass&&) = delete;

    [[nodiscard]] const std::string& getName() const noexcept { return _name; }

    [[nodiscard]]       VulkanShaderProgram*& getShaderProgram()       noexcept { return _shaderProgram; }
    [[nodiscard]] const VulkanShaderProgram*  getShaderProgram() const noexcept { return _shaderProgram; }

    [[nodiscard]]       VulkanPipelineDescriptor& getPipelineDescriptor()       noexcept { return _pipelineDescriptor; }
    [[nodiscard]] const VulkanPipelineDescriptor& getPipelineDescriptor() const noexcept { return _pipelineDescriptor; }

    [[nodiscard]] const VulkanGraphicsPipeline* getPipeline() const noexcept { return _pipeline; }

    [[nodiscard]] vk::PipelineBindPoint getBindPoint() const noexcept { return _bindPoint; }

    [[nodiscard]]       DescriptorSetsMap& getDescriptorSets()       noexcept { return _descriptorSets; }
    [[nodiscard]] const DescriptorSetsMap& getDescriptorSets() const noexcept { return _descriptorSets; }

    [[nodiscard]] DescriptorManagersMap& getDescriptorManagers() noexcept { return _descriptorManagers; }

    [[nodiscard]] VulkanRenderPassAttachment* getDepthAttachment() const noexcept { return _depthAttachment; }

    [[nodiscard]] std::vector<std::unique_ptr<VulkanRenderPassAttachment>>& getColorAttachments() noexcept {
        return _colorAttachments;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<VulkanRenderPassAttachment>>& getColorAttachments() const noexcept {
        return _colorAttachments;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<VulkanDrawCall>>& getDrawCalls() const noexcept {
        return _drawCalls;
    }

    [[nodiscard]]       std::vector<VulkanPassTransition>& getTransitions()       noexcept { return _transitions; }
    [[nodiscard]] const std::vector<VulkanPassTransition>& getTransitions() const noexcept { return _transitions; }

    VulkanRenderPass& setName(const std::string& name) noexcept { _name = name; return *this; }

    VulkanRenderPass& setPipelineDescriptor(const VulkanPipelineDescriptor& pipelineDescriptor) noexcept {
        _pipelineDescriptor = pipelineDescriptor;
        return *this;
    }

    VulkanRenderPass& setPipeline(const VulkanGraphicsPipeline* pipeline) noexcept {
        _pipeline = pipeline;
        return *this;
    }

    VulkanRenderPass& setBindPoint(const vk::PipelineBindPoint bindPoint) noexcept {
        _bindPoint = bindPoint;
        return *this;
    }

    VulkanRenderPass& setDepthAttachment(VulkanRenderPassAttachment* depthAttachment) noexcept {
        _depthAttachment = depthAttachment;
        return *this;
    }

    VulkanRenderPass& addColorAttachment(const VulkanRenderPassAttachment& colorAttachment) noexcept {
        _colorAttachments.push_back(std::make_unique<VulkanRenderPassAttachment>(colorAttachment));
        return *this;
    }

    VulkanRenderPass& addColorAttachmentAtIndex(const long index, const VulkanRenderPassAttachment& colorAttachment) {
        _colorAttachments.insert(
            _colorAttachments.begin() + index, std::make_unique<VulkanRenderPassAttachment>(colorAttachment)
        );
        return *this;
    }

    VulkanRenderPass& addDrawCall(std::unique_ptr<VulkanDrawCall> drawCall) noexcept {
        _drawCalls.push_back(std::move(drawCall));
        return *this;
    }

    VulkanRenderPass& addTransition(const VulkanPassTransition& transition) {
        if (const auto it = std::ranges::find(_transitions, transition); it == _transitions.end()) {
            _transitions.push_back(transition);
        }
        return *this;
    }

private:
    std::string _name = "Undefined_Pass";

    VulkanShaderProgram* _shaderProgram = nullptr;

    VulkanPipelineDescriptor _pipelineDescriptor{};

    const VulkanGraphicsPipeline* _pipeline  = nullptr;
    vk::PipelineBindPoint         _bindPoint = vk::PipelineBindPoint::eGraphics;

    DescriptorSetsMap     _descriptorSets{};
    DescriptorManagersMap _descriptorManagers{};

    VulkanRenderPassAttachment* _depthAttachment = nullptr;

    std::vector<std::unique_ptr<VulkanRenderPassAttachment>> _colorAttachments{};

    std::vector<std::unique_ptr<VulkanDrawCall>> _drawCalls{};

    std::vector<VulkanPassTransition> _transitions{};
};

#endif // NOBLEENGINE_VULKANRENDERPASS_H
