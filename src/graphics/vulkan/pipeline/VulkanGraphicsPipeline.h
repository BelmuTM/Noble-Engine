#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanPipelineDescriptor.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

class VulkanGraphicsPipeline {
public:
    VulkanGraphicsPipeline()  = default;
    ~VulkanGraphicsPipeline() = default;

    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&)            = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&)            = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

    [[nodiscard]] bool create(
        const vk::Device&       device,
        const VulkanRenderPass& pass,
        std::string&            errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] vk::Pipeline handle() const noexcept { return _pipeline; }

    [[nodiscard]] const vk::PipelineLayout& getLayout() const noexcept { return _pipelineLayout; }

private:
    [[nodiscard]] bool createPipelineLayout(
        const vk::Device&               device,
        const VulkanPipelineDescriptor& descriptor,
        std::string&                    errorMessage
    );

    [[nodiscard]] bool createPipeline(
        const vk::Device&       device,
        const VulkanRenderPass& pass,
        std::string&            errorMessage
    );

    vk::Device _device{};

    vk::Pipeline       _pipeline{};
    vk::PipelineLayout _pipelineLayout{};
};
