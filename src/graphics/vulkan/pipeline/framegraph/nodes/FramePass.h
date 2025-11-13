#pragma once
#ifndef NOBLEENGINE_FRAMEPASS_H
#define NOBLEENGINE_FRAMEPASS_H

#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/DrawCall.h"
#include "graphics/vulkan/pipeline/framegraph/nodes/FramePassAttachment.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

#include <memory>

struct FramePass {
    std::string name = "Undefined_Pass";

    const VulkanGraphicsPipeline* pipeline  = nullptr;
    vk::PipelineBindPoint         bindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<FramePassAttachment> colorAttachments{};
    FramePassAttachment              depthAttachment{};

    std::vector<FrameResource*> reads{};
    std::vector<FrameResource*> writes{};

    std::vector<std::unique_ptr<DrawCall>> drawCalls{};

    FramePass() = default;

    FramePass(FramePass&&)           = default;
    FramePass& operator=(FramePass&&) = default;

    FramePass(const FramePass&)            = delete;
    FramePass& operator=(const FramePass&) = delete;

    FramePass& setName(const std::string& _name) { name = _name; return *this; }

    FramePass& setPipeline(const VulkanGraphicsPipeline* _pipeline) { pipeline = _pipeline; return *this; }

    FramePass& setBindPoint(const vk::PipelineBindPoint _bindPoint) { bindPoint = _bindPoint; return *this; }

    FramePass& addColorAttachment(const FramePassAttachment& colorAttachment) {
        colorAttachments.push_back(colorAttachment); return *this;
    }

    FramePass& setDepthAttachment(const FramePassAttachment& _depthAttachment) {
        depthAttachment = _depthAttachment; return *this;
    }

    FramePass& addRead(FrameResource* read) { reads.push_back(read); return *this; }

    FramePass& addWrite(FrameResource* write) { writes.push_back(write); return *this; }

    FramePass& addDrawCall(std::unique_ptr<DrawCall> drawCall) {
        drawCalls.push_back(std::move(drawCall));
        return *this;
    }

    FramePass& addObjectDrawCall(const VulkanRenderObject* renderObject) {
        // Each submesh requires its own draw call
        for (const auto& submesh : renderObject->submeshes) {
            auto verticesDraw = std::make_unique<DrawCallPushConstant<ObjectDataGPU>>();
            verticesDraw->setMesh(*submesh.mesh);

            verticesDraw->setDescriptorResolver(
                [&submesh](const FrameContext& frame) {
                    return std::vector{submesh.descriptorSets->getSets().at(frame.frameIndex)};
                }
            );

            verticesDraw->setPushConstantResolver([renderObject](const FrameContext&) { return renderObject->data; });

            addDrawCall(std::move(verticesDraw));
        }

        return *this;
    }
};

#endif // NOBLEENGINE_FRAMEPASS_H
