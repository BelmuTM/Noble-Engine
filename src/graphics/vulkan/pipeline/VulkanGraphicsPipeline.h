#pragma once
#ifndef NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
#define NOBLEENGINE_VULKANGRAPHICSPIPELINE_H

#include "../core/VulkanSwapchain.h"

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

class VulkanGraphicsPipeline {
public:
    VulkanGraphicsPipeline()  = default;
    ~VulkanGraphicsPipeline() = default;

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
};

#endif //NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
