#pragma once
#ifndef NOBLEENGINE_VULKANFRAMEGRAPH_H
#define NOBLEENGINE_VULKANFRAMEGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "VulkanGraphicsPipeline.h"
#include "graphics/vulkan/resources/VulkanMeshManager.h"

#include <optional>

enum FrameResourceType { Texture, Buffer };

struct FrameResource {
    std::string       path;
    FrameResourceType type;
    vk::Image         image;
    vk::ImageView     imageView;
    vk::ImageLayout   layout = vk::ImageLayout::eUndefined;
};

struct DrawCall {
    const VulkanGraphicsPipeline* pipeline;
    VulkanMesh                    mesh;

    std::optional<std::vector<vk::DescriptorSet>> descriptorSets;

    std::optional<vk::Viewport> viewport;
    std::optional<vk::Rect2D>   scissor;
};

struct FramePass {
    std::string           name;
    vk::PipelineBindPoint bindPoint = vk::PipelineBindPoint::eGraphics;
    vk::RenderingInfo     renderingInfo;

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

    void executePass(const FramePass& pass, vk::CommandBuffer commandBuffer) const;

    [[nodiscard]] std::vector<FramePass> getPasses() const { return _passes; }

private:
    const VulkanMeshManager* _meshManager = nullptr;

    std::vector<FramePass>     _passes;
    std::vector<FrameResource> _resources;
};

#endif //NOBLEENGINE_VULKANFRAMEGRAPH_H