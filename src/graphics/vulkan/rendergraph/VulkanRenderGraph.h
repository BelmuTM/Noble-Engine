#pragma once

#include "core/debug/ErrorHandling.h"

#include "VulkanFrameContext.h"

#include "nodes/VulkanPass.h"

#include <memory>
#include <vector>

class VulkanGraphResourceManager;

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

    Expected<void> execute(
        vk::CommandBuffer                 commandBuffer,
        const VulkanFrameContext&         frameContext,
        const VulkanGraphResourceManager& resources
    ) const;

    static Expected<void> executePass(
        vk::CommandBuffer                 commandBuffer,
        const VulkanFrameContext&         frameContext,
        const VulkanGraphResourceManager& resources,
        const VulkanPass&         pass
    );

    [[nodiscard]]       std::vector<std::unique_ptr<VulkanPass>>& getPasses()       noexcept { return _passes; }
    [[nodiscard]] const std::vector<std::unique_ptr<VulkanPass>>& getPasses() const noexcept { return _passes; }

    void addPass(std::unique_ptr<VulkanPass> pass) {
        _passes.push_back(std::move(pass));
    }

private:
    std::vector<std::unique_ptr<VulkanPass>> _passes{};
};
