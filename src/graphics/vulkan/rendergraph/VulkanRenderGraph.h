#pragma once

#include "core/debug/ErrorHandling.h"

#include "VulkanFrameContext.h"

#include "nodes/VulkanGraphicsPass.h"

#include <memory>
#include <vector>

class VulkanRenderResourceManager;

class VulkanRenderGraph {
public:
    VulkanRenderGraph()  = default;
    ~VulkanRenderGraph() = default;

    VulkanRenderGraph(const VulkanRenderGraph&)            = delete;
    VulkanRenderGraph& operator=(const VulkanRenderGraph&) = delete;

    VulkanRenderGraph(VulkanRenderGraph&&)            = delete;
    VulkanRenderGraph& operator=(VulkanRenderGraph&&) = delete;

    [[nodiscard]] Expected<void> create() noexcept;

    void destroy() noexcept;

    Expected<void> execute(vk::CommandBuffer commandBuffer, const VulkanFrameContext& frameContext) const;

    static Expected<void> executePass(
        vk::CommandBuffer commandBuffer, const VulkanFrameContext& frameContext, const VulkanGraphicsPass& pass
    ) ;

    [[nodiscard]]       std::vector<std::unique_ptr<VulkanGraphicsPass>>& getPasses()       noexcept { return _passes; }
    [[nodiscard]] const std::vector<std::unique_ptr<VulkanGraphicsPass>>& getPasses() const noexcept { return _passes; }

    void addPass(std::unique_ptr<VulkanGraphicsPass> pass) {
        _passes.push_back(std::move(pass));
    }

private:
    std::vector<std::unique_ptr<VulkanGraphicsPass>> _passes{};
};
