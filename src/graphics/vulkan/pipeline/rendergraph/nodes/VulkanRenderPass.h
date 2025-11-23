#pragma once
#ifndef NOBLEENGINE_VULKANRENDERPASS_H
#define NOBLEENGINE_VULKANRENDERPASS_H

#include "VulkanDrawCall.h"
#include "VulkanRenderPassAttachment.h"

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/pipeline/rendergraph/VulkanRenderResources.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

#include <memory>

class VulkanRenderPass {
public:
    VulkanRenderPass()  = default;
    ~VulkanRenderPass() = default;

    VulkanRenderPass(const VulkanRenderPass&)            = default;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = default;

    VulkanRenderPass(VulkanRenderPass&&)            = delete;
    VulkanRenderPass& operator=(VulkanRenderPass&&) = delete;

    [[nodiscard]] const std::string& getName() const noexcept { return _name; }

    [[nodiscard]] const VulkanPipelineDescriptor& getPipelineDescriptor() const noexcept { return _pipelineDescriptor; }

    [[nodiscard]] const VulkanGraphicsPipeline* getPipeline() const noexcept { return _pipeline; }

    [[nodiscard]] vk::PipelineBindPoint getBindPoint() const noexcept { return _bindPoint; }

    [[nodiscard]] const VulkanRenderPassAttachment& getDepthAttachment() const noexcept { return _depthAttachment; }

    [[nodiscard]] std::vector<std::unique_ptr<VulkanRenderPassAttachment>>& getColorAttachments() noexcept {
        return _colorAttachments;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<VulkanDrawCall>>& getDrawCalls() const noexcept {
        return _drawCalls;
    }

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

    VulkanRenderPass& setDepthAttachment(const VulkanRenderPassAttachment& depthAttachment) noexcept {
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

    VulkanRenderPass& addObjectDrawCall(const VulkanRenderObject* renderObject);

    [[nodiscard]] bool createColorAttachments(
        const VulkanShaderProgram& shaderProgram,
        const VulkanImageManager&  imageManager,
        VulkanFrameResources&      frameResources,
        VulkanRenderResources&     renderResources,
        std::string&               errorMessage
    );

    std::vector<VulkanRenderPassResource*> _sampledInputs;

private:
    std::string _name = "Undefined_Pass";

    VulkanPipelineDescriptor _pipelineDescriptor{};

    const VulkanGraphicsPipeline* _pipeline  = nullptr;
    vk::PipelineBindPoint         _bindPoint = vk::PipelineBindPoint::eGraphics;

    VulkanRenderPassAttachment _depthAttachment{};

    std::vector<std::unique_ptr<VulkanRenderPassAttachment>> _colorAttachments{};

    std::vector<std::unique_ptr<VulkanDrawCall>> _drawCalls{};
};

#endif // NOBLEENGINE_VULKANRENDERPASS_H
