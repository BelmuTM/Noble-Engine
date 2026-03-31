#pragma once

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"

class VulkanFrameDraws {
public:
    VulkanFrameDraws()  = default;
    ~VulkanFrameDraws() = default;

    VulkanFrameDraws(const VulkanFrameDraws&)            = delete;
    VulkanFrameDraws& operator=(const VulkanFrameDraws&) = delete;

    VulkanFrameDraws(VulkanFrameDraws&&)            = delete;
    VulkanFrameDraws& operator=(VulkanFrameDraws&&) = delete;

    void cullDraws(const std::vector<std::unique_ptr<VulkanRenderPass>>& passes, const FrameUniforms& uniforms);

    [[nodiscard]] const std::vector<const VulkanDrawCall*>& getDrawCalls(const VulkanRenderPass* pass) const {
        return _visibleDrawCalls.at(pass);
    }

private:
    std::unordered_map<const VulkanRenderPass*, std::vector<const VulkanDrawCall*>> _visibleDrawCalls{};
};
