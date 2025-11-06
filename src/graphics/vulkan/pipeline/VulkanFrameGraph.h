#pragma once
#ifndef NOBLEENGINE_VULKANFRAMEGRAPH_H
#define NOBLEENGINE_VULKANFRAMEGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "VulkanGraphicsPipeline.h"

#include "graphics/vulkan/resources/mesh/VulkanMeshManager.h"
#include "graphics/vulkan/resources/ubo/FrameUniformBuffer.h"

#include <functional>
#include <optional>

static constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

struct FrameContext {
    uint32_t          frameIndex = 0;
    vk::CommandBuffer cmdBuffer{};
    vk::ImageView     swapchainImageView{};
    vk::Extent2D      extent{};

    std::vector<vk::DescriptorSet> frameDescriptors;

    FrameContext& setFrameIndex(const uint32_t _frameIndex) { frameIndex = _frameIndex; return *this; }

    FrameContext& setCommandBuffer(const vk::CommandBuffer _cmdBuffer) { cmdBuffer = _cmdBuffer; return *this; }

    FrameContext& setSwapchainImageView(const vk::ImageView _swapchainImageView) {
        swapchainImageView = _swapchainImageView; return *this;
    }

    FrameContext& setExtent(const vk::Extent2D _extent) { extent = _extent; return *this; }
};

enum FrameResourceType { Texture, Buffer };

struct FrameResource {
    std::string       path = "UnknownResource";
    FrameResourceType type = Texture;
    vk::Image         image;
    vk::ImageView     imageView;
    vk::ImageLayout   layout = vk::ImageLayout::eUndefined;

    std::function<vk::ImageView(const FrameContext&)> resolveImageView;

    FrameResource& setPath(const std::string& _path) { path = _path; return *this; }

    FrameResource& setType(const FrameResourceType _type) { type = _type; return *this; }

    FrameResource& setImage(const vk::Image _image) { image = _image; return *this; }

    FrameResource& setImageView(const vk::ImageView _imageView) { imageView = _imageView; return *this; }

    FrameResource& setLayout(const vk::ImageLayout _layout) { layout = _layout; return *this; }

    FrameResource& setResolveImageView(const std::function<vk::ImageView(const FrameContext&)>& _resolveImageView) {
        resolveImageView = _resolveImageView;
        return *this;
    }
};

struct FramePassAttachment {
    FrameResource resource;

    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;

    vk::ClearValue clearValue = clearColor;

    FramePassAttachment& setResource(const FrameResource& _resource) { resource = _resource; return *this; }

    FramePassAttachment& setLoadOp(const vk::AttachmentLoadOp _loadOp) { loadOp = _loadOp; return *this; }

    FramePassAttachment& setStoreOp(const vk::AttachmentStoreOp _storeOp) { storeOp = _storeOp; return *this; }

    FramePassAttachment& setClearValue(const vk::ClearValue _clearValue) { clearValue = _clearValue; return *this; }
};

struct DrawCall {
    VulkanMesh mesh;

    std::function<std::vector<vk::DescriptorSet>(const FrameContext& frame)> descriptorResolver;

    std::optional<vk::Viewport> viewport;
    std::optional<vk::Rect2D>   scissor;

    [[nodiscard]] vk::Viewport resolveViewport(const FrameContext& frame) const {
        if (viewport) return *viewport;
        return vk::Viewport{
            0.0f, 0.0f, static_cast<float>(frame.extent.width), static_cast<float>(frame.extent.height), 0.0f, 1.0f
        };
    }

    [[nodiscard]] vk::Rect2D resolveScissor(const FrameContext& frame) const {
        if (scissor) return *scissor;
        return {vk::Offset2D(0, 0), frame.extent};
    }

    DrawCall& setMesh(const VulkanMesh& _mesh) { mesh = _mesh; return *this; }

    DrawCall& setDescriptorResolver(
        const std::function<std::vector<vk::DescriptorSet>(const FrameContext& frame)>& _descriptorResolver
    ) {
        descriptorResolver = _descriptorResolver; return *this;
    }

    DrawCall& setViewport(const vk::Viewport& _viewport) { viewport = _viewport; return *this; }

    DrawCall& setScissor(const vk::Rect2D _scissor) { scissor = _scissor; return *this; }
};

struct FramePass {
    std::string name = "UndefinedPass";

    const VulkanGraphicsPipeline* pipeline  = nullptr;
    vk::PipelineBindPoint         bindPoint = vk::PipelineBindPoint::eGraphics;

    std::vector<FramePassAttachment> colorAttachments{};
    FramePassAttachment              depthAttachment{};

    std::vector<FrameResource*> reads{};
    std::vector<FrameResource*> writes{};

    std::vector<DrawCall> drawCalls{};

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

    FramePass& addDrawCall(const DrawCall& drawCall) { drawCalls.push_back(drawCall); return *this; }
};

class VulkanFrameGraph {
public:
    VulkanFrameGraph()  = default;
    ~VulkanFrameGraph() = default;

    VulkanFrameGraph(const VulkanFrameGraph&)            = delete;
    VulkanFrameGraph& operator=(const VulkanFrameGraph&) = delete;

    VulkanFrameGraph(VulkanFrameGraph&&)            = delete;
    VulkanFrameGraph& operator=(VulkanFrameGraph&&) = delete;

    [[nodiscard]] bool create(const VulkanMeshManager& meshManager, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void execute(const FrameContext& frame) const;

    void addPass(FramePass pass) { _passes.push_back(std::move(pass)); }

    void executePass(const FramePass& pass, const FrameContext& frame) const;

    [[nodiscard]] std::vector<FramePass> getPasses() const { return _passes; }

private:
    const VulkanMeshManager* _meshManager = nullptr;

    std::vector<FramePass>     _passes;
    std::vector<FrameResource> _resources;
};

#endif //NOBLEENGINE_VULKANFRAMEGRAPH_H
