#pragma once
#ifndef NOBLEENGINE_VULKANRENDERGRAPH_H
#define NOBLEENGINE_VULKANRENDERGRAPH_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/VulkanFrameResources.h"
#include "graphics/vulkan/resources/meshes/VulkanMeshManager.h"

#include "graphics/vulkan/pipeline/rendergraph/nodes/VulkanRenderPass.h"
#include "graphics/vulkan/pipeline/rendergraph/nodes/VulkanRenderPassResource.h"

class VulkanRenderGraph {
public:
    VulkanRenderGraph()  = default;
    ~VulkanRenderGraph() = default;

    VulkanRenderGraph(const VulkanRenderGraph&)            = delete;
    VulkanRenderGraph& operator=(const VulkanRenderGraph&) = delete;

    VulkanRenderGraph(VulkanRenderGraph&&)            = delete;
    VulkanRenderGraph& operator=(VulkanRenderGraph&&) = delete;

    [[nodiscard]] bool create(
        const VulkanMeshManager&    meshManager,
        const VulkanFrameResources& frame,
        vk::QueryPool               queryPool,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    void execute(vk::CommandBuffer commandBuffer) const;

    void addPass(std::unique_ptr<VulkanRenderPass> pass) {
        _passes.push_back(std::move(pass));
    }

    void attachSwapchainOutput(const VulkanSwapchain& swapchain) const;

    void executePass(vk::CommandBuffer commandBuffer, const VulkanRenderPass* pass) const;

    [[nodiscard]] std::vector<std::unique_ptr<VulkanRenderPass>>& getPasses() { return _passes; }

private:
    const VulkanMeshManager*    _meshManager = nullptr;
    const VulkanFrameResources* _frame       = nullptr;

    vk::QueryPool _queryPool{};

    std::vector<std::unique_ptr<VulkanRenderPass>> _passes{};
    std::vector<VulkanRenderPassResource>          _resources{};
};

#endif //NOBLEENGINE_VULKANRENDERGRAPH_H
