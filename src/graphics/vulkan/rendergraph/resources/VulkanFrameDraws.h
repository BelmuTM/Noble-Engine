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

    [[nodiscard]] bool create(const VulkanFrameResources& frameResources, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    void cullDraws(const std::vector<std::unique_ptr<VulkanRenderPass>>& passes);

    [[nodiscard]] const std::vector<const VulkanDrawCall*>& getDrawCalls(const VulkanRenderPass* pass) const {
        return _visibleDrawCalls.at(pass);
    }

private:
    const VulkanFrameResources* _frameResources = nullptr;

    std::unordered_map<const VulkanRenderPass*, std::vector<const VulkanDrawCall*>> _visibleDrawCalls{};
};
