#pragma once
#ifndef NOBLEENGINE_VULKANFRAMEGRAPH_H
#define NOBLEENGINE_VULKANFRAMEGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/FrameContext.h"
#include "graphics/vulkan/pipeline/framegraph/nodes/FramePass.h"
#include "graphics/vulkan/pipeline/framegraph/nodes/FrameResource.h"

class VulkanFrameGraph {
public:
    VulkanFrameGraph()  = default;
    ~VulkanFrameGraph() = default;

    VulkanFrameGraph(const VulkanFrameGraph&)            = delete;
    VulkanFrameGraph& operator=(const VulkanFrameGraph&) = delete;

    VulkanFrameGraph(VulkanFrameGraph&&)            = delete;
    VulkanFrameGraph& operator=(VulkanFrameGraph&&) = delete;

    [[nodiscard]] bool create(
        const VulkanMeshManager& meshManager, vk::QueryPool queryPool, std::string& errorMessage
    ) noexcept;

    void destroy() noexcept;

    void execute(const FrameContext& frame) const;

    void addPass(FramePass pass) {
        _passes.push_back(std::move(pass));
    }

    void executePass(const FramePass& pass, const FrameContext& frame) const;

    [[nodiscard]] const std::vector<FramePass>& getPasses() const { return _passes; }

private:
    const VulkanMeshManager* _meshManager = nullptr;

    vk::QueryPool _queryPool{};

    std::vector<FramePass>     _passes{};
    std::vector<FrameResource> _resources{};
};

#endif //NOBLEENGINE_VULKANFRAMEGRAPH_H
