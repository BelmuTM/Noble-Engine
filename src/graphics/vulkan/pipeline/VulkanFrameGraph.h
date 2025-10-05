#pragma once
#ifndef NOBLEENGINE_VULKANFRAMEGRAPH_H
#define NOBLEENGINE_VULKANFRAMEGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "VulkanGraphicsPipeline.h"
#include "graphics/vulkan/resources/VulkanMeshManager.h"

#include <functional>
#include <optional>

static constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct FrameContext {
    uint32_t          frameIndex;
    vk::CommandBuffer cmdBuffer;
    vk::ImageView     swapchainImageView;
    vk::Extent2D      extent;
};

enum FrameResourceType { Texture, Buffer };

struct FrameResource {
    std::string       path = "UnknownResource";
    FrameResourceType type = Texture;
    vk::Image         image;
    vk::ImageView     imageView;
    vk::ImageLayout   layout = vk::ImageLayout::eUndefined;

    std::function<vk::ImageView(const FrameContext&)> resolveImageView;
};

struct FramePassAttachment {
    FrameResource resource;

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = clearColor;
};

struct DrawCall {
    const VulkanGraphicsPipeline* pipeline;
    VulkanMesh                    mesh;

    std::function<std::vector<vk::DescriptorSet>(const FrameContext& frame)> descriptorResolver;

    std::optional<vk::Viewport> viewport;
    std::optional<vk::Rect2D>   scissor;

    vk::Viewport resolveViewport(const FrameContext& frame) const {
        if (viewport) return *viewport;
        return vk::Viewport{
            0.0f, 0.0f, static_cast<float>(frame.extent.width), static_cast<float>(frame.extent.height), 0.0f, 1.0f
        };
    }

    vk::Rect2D resolveScissor(const FrameContext& frame) const {
        if (scissor) return *scissor;
        return vk::Rect2D(vk::Offset2D(0, 0), frame.extent);
    }
};

struct FramePass {
    std::string name;

    vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<FramePassAttachment> colorAttachments;

    std::vector<FrameResource*> reads  = {};
    std::vector<FrameResource*> writes = {};

    std::vector<DrawCall> drawCalls;
};

class VulkanFrameGraph {
public:
    VulkanFrameGraph()  = default;
    ~VulkanFrameGraph() = default;

    VulkanFrameGraph(const VulkanFrameGraph&)            = delete;
    VulkanFrameGraph& operator=(const VulkanFrameGraph&) = delete;
    VulkanFrameGraph(VulkanFrameGraph&&)                 = delete;
    VulkanFrameGraph& operator=(VulkanFrameGraph&&)      = delete;

    [[nodiscard]] bool create(const VulkanMeshManager& meshManager, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void addPass(FramePass pass) { _passes.push_back(std::move(pass)); }

    void executePass(const FramePass& pass, const FrameContext& frame) const;

    [[nodiscard]] std::vector<FramePass> getPasses() const { return _passes; }

    void execute(const FrameContext& frame) const;

private:
    const VulkanMeshManager* _meshManager = nullptr;

    std::vector<FramePass>     _passes;
    std::vector<FrameResource> _resources;
};

#endif //NOBLEENGINE_VULKANFRAMEGRAPH_H