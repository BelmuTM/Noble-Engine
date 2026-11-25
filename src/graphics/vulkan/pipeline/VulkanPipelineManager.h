#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/compute/VulkanComputePipeline.h"
#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgram.h"

#include <memory>
#include <unordered_map>

class VulkanPipelineManager {
public:
    template<typename PipelineType>
    using PipelinesMap = std::unordered_map<std::string, std::unique_ptr<PipelineType>>;

    VulkanPipelineManager()  = default;
    ~VulkanPipelineManager() = default;

    VulkanPipelineManager(const VulkanPipelineManager&)            = delete;
    VulkanPipelineManager& operator=(const VulkanPipelineManager&) = delete;

    VulkanPipelineManager(VulkanPipelineManager&&)            = delete;
    VulkanPipelineManager& operator=(VulkanPipelineManager&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<const VulkanGraphicsPipeline*> getOrCreate(
        const VulkanGraphicsPipelineDefinition& definition,
        const VulkanShaderProgram&              program,
        VulkanGraphicsPipelineTarget&           target
    );

    [[nodiscard]] const PipelinesMap<VulkanGraphicsPipeline>& getGraphicsPipelines() const noexcept {
        return _graphicsPipelines;
    }

    [[nodiscard]] const PipelinesMap<VulkanComputePipeline>& getComputePipelines() const noexcept {
        return _computePipelines;
    }

private:
    vk::Device _device{};

    PipelinesMap<VulkanGraphicsPipeline> _graphicsPipelines{};
    PipelinesMap<VulkanComputePipeline>  _computePipelines{};
};
