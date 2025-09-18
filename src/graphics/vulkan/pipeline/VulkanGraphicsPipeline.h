#pragma once
#ifndef NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
#define NOBLEENGINE_VULKANGRAPHICSPIPELINE_H

#include "graphics/vulkan/core/VulkanSwapchain.h"
#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanGraphicsPipeline {
public:
    VulkanGraphicsPipeline()  = default;
    ~VulkanGraphicsPipeline() = default;

    // Implicit conversion operator
    operator vk::Pipeline() const { return pipeline; }

    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&)            = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;
    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&)                 = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&)      = delete;

    [[nodiscard]] bool create(
        const vk::Device* device, const VulkanSwapchain& swapchain, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

private:
    const vk::Device*      _device    = nullptr;
    const VulkanSwapchain* _swapchain = nullptr;

    vk::Pipeline       pipeline{};
    vk::PipelineLayout pipelineLayout{};

    bool createPipelineLayout(std::string& errorMessage);
    bool createPipeline(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
