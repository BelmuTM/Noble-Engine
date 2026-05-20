#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanPipelineDescriptor.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

class VulkanComputePipeline {
public:
    VulkanComputePipeline()  = default;
    ~VulkanComputePipeline() = default;

    VulkanComputePipeline(const VulkanComputePipeline&)            = delete;
    VulkanComputePipeline& operator=(const VulkanComputePipeline&) = delete;

    VulkanComputePipeline(VulkanComputePipeline&&)            = delete;
    VulkanComputePipeline& operator=(VulkanComputePipeline&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device, const VulkanRenderPass& pass) noexcept;

    void destroy() noexcept;

    [[nodiscard]] vk::Pipeline handle() const noexcept { return _pipeline; }

    [[nodiscard]] const vk::PipelineLayout& getLayout() const noexcept { return _pipelineLayout; }

    [[nodiscard]] const vk::PipelineBindPoint& getBindPoint() const noexcept { return _pipelineBindPoint; }

private:
    [[nodiscard]] Expected<void> createPipelineLayout(
        const vk::Device& device, const VulkanPipelineDescriptor& descriptor
    );

    [[nodiscard]] Expected<void> createComputePipeline(const vk::Device& device, const VulkanRenderPass& pass);

    vk::Device _device{};

    vk::Pipeline       _pipeline{};
    vk::PipelineLayout _pipelineLayout{};

    vk::PipelineBindPoint _pipelineBindPoint = vk::PipelineBindPoint::eCompute;
};
