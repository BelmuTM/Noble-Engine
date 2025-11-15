#pragma once
#ifndef NOBLEENGINE_FRAMEPASS_H
#define NOBLEENGINE_FRAMEPASS_H

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanDrawCall.h"
#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFramePassAttachment.h"

#include "graphics/vulkan/resources/images/VulkanImageManager.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

#include <memory>

class VulkanFramePass {
public:
    VulkanFramePass()  = default;
    ~VulkanFramePass() = default;

    VulkanFramePass(const VulkanFramePass&)            = default;
    VulkanFramePass& operator=(const VulkanFramePass&) = default;

    VulkanFramePass(VulkanFramePass&&)            = delete;
    VulkanFramePass& operator=(VulkanFramePass&&) = delete;

    [[nodiscard]] const std::string& getName() const noexcept { return _name; }

    [[nodiscard]] const VulkanGraphicsPipeline* getPipeline() const noexcept { return _pipeline; }

    [[nodiscard]] vk::PipelineBindPoint getBindPoint() const noexcept { return _bindPoint; }

    [[nodiscard]] const std::vector<VulkanFramePassAttachment>& getColorAttachments() const noexcept {
        return _colorAttachments;
    }

    [[nodiscard]] const VulkanFramePassAttachment& getDepthAttachment() const noexcept { return _depthAttachment; }

    [[nodiscard]] const std::vector<VulkanFramePassResource>& getReads() const noexcept { return _reads; }
    [[nodiscard]] const std::vector<VulkanFramePassResource>& getWrites() const noexcept { return _writes; }

    [[nodiscard]] const std::vector<std::unique_ptr<VulkanDrawCall>>& getDrawCalls() const noexcept {
        return _drawCalls;
    }

    VulkanFramePass& setName(const std::string& name) noexcept { _name = name; return *this; }

    VulkanFramePass& setPipeline(const VulkanGraphicsPipeline* pipeline) noexcept {
        _pipeline = pipeline; return *this;
    }

    VulkanFramePass& setBindPoint(const vk::PipelineBindPoint bindPoint) noexcept {
        _bindPoint = bindPoint; return *this;
    }

    VulkanFramePass& addColorAttachment(const VulkanFramePassAttachment& colorAttachment) noexcept {
        _colorAttachments.push_back(colorAttachment);
        return *this;
    }

    VulkanFramePass& setDepthAttachment(const VulkanFramePassAttachment& depthAttachment) noexcept {
        _depthAttachment = depthAttachment;
        return *this;
    }

    VulkanFramePass& addRead(const VulkanFramePassResource& read) noexcept { _reads.push_back(read); return *this; }
    VulkanFramePass& addWrite(const VulkanFramePassResource& write) noexcept { _writes.push_back(write); return *this; }

    VulkanFramePass& addDrawCall(std::unique_ptr<VulkanDrawCall> drawCall) noexcept {
        _drawCalls.push_back(std::move(drawCall));
        return *this;
    }

    VulkanFramePass& addObjectDrawCall(const VulkanRenderObject* renderObject);

    /*
    [[nodiscard]] bool createColorAttachments(
        const VulkanShaderProgram& shaderProgram,
        VulkanImageManager&        imageManager,
        const VulkanFrameContext&  frame,
        std::string&               errorMessage
    );
    */

private:
    std::string _name = "Undefined_Pass";

    const VulkanGraphicsPipeline* _pipeline  = nullptr;
    vk::PipelineBindPoint         _bindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<VulkanFramePassAttachment> _colorAttachments{};
    VulkanFramePassAttachment              _depthAttachment{};

    std::vector<VulkanFramePassResource> _reads{};
    std::vector<VulkanFramePassResource> _writes{};

    std::vector<std::unique_ptr<VulkanDrawCall>> _drawCalls{};
};

#endif // NOBLEENGINE_FRAMEPASS_H
