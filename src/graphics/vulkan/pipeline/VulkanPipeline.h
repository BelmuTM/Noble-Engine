#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanPipelineDescriptor.h"

class VulkanPipeline {
public:
    VulkanPipeline()          = default;
    virtual ~VulkanPipeline() = default;

    VulkanPipeline(const VulkanPipeline&)            = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    VulkanPipeline(VulkanPipeline&&)            = delete;
    VulkanPipeline& operator=(VulkanPipeline&&) = delete;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> createPipelineLayout(
        const vk::Device& device, const VulkanPipelineDescriptor& descriptor
    );

    [[nodiscard]] vk::Pipeline handle() const noexcept { return _pipeline; }

    [[nodiscard]] const vk::PipelineLayout& getLayout() const noexcept { return _pipelineLayout; }

protected:
    vk::Device _device{};

    vk::Pipeline       _pipeline{};
    vk::PipelineLayout _pipelineLayout{};
};
