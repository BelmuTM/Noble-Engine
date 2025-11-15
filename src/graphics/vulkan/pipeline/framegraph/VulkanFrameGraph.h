#pragma once
#ifndef NOBLEENGINE_VULKANFRAMEGRAPH_H
#define NOBLEENGINE_VULKANFRAMEGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFrameContext.h"
#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFramePass.h"
#include "graphics/vulkan/pipeline/framegraph/nodes/VulkanFramePassResource.h"

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

    void execute(const VulkanFrameContext& frame) const;

    void addPass(std::unique_ptr<VulkanFramePass> pass) {
        _passes.push_back(std::move(pass));
    }

    void attachSwapchainOutput() const;

    void executePass(const VulkanFramePass* pass, const VulkanFrameContext& frame) const;

    [[nodiscard]] const std::vector<std::unique_ptr<VulkanFramePass>>& getPasses() const { return _passes; }

private:
    const VulkanMeshManager* _meshManager = nullptr;

    vk::QueryPool _queryPool{};

    std::vector<std::unique_ptr<VulkanFramePass>> _passes;
    std::vector<VulkanFramePassResource>              _resources{};
};

#endif //NOBLEENGINE_VULKANFRAMEGRAPH_H
