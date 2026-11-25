#include "VulkanPipelineManager.h"

#include <ranges>

Expected<void> VulkanPipelineManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanPipelineManager::destroy() noexcept {
    for (const auto& graphicsPipeline : _graphicsPipelines | std::views::values) {
        graphicsPipeline->destroy();
    }

    _graphicsPipelines.clear();

    for (const auto& computePipeline : _computePipelines | std::views::values) {
        computePipeline->destroy();
    }

    _computePipelines.clear();

    _device = VK_NULL_HANDLE;
}

Expected<const VulkanGraphicsPipeline*> VulkanPipelineManager::getOrCreate(
    const VulkanGraphicsPipelineDefinition& definition,
    const VulkanShaderProgram&              program,
    VulkanGraphicsPipelineTarget&           target
) {

    const auto pipelineIt = _graphicsPipelines.find(definition.name);

    if (pipelineIt != _graphicsPipelines.end()) {
        const VulkanGraphicsPipeline* pipelinePtr = pipelineIt->second.get();
        return Expected(pipelinePtr);
    }

    target.layout.pushConstantRanges = program.getPushConstants();

    const VulkanGraphicsPipelineDescriptor descriptor(
        program.getStages(),
        definition.state,
        target.layout,
        target.colorAttachmentFormats,
        target.depthAttachmentFormat,
        target.depthWrite
    );

    auto [cachedPipeline, inserted] = _graphicsPipelines.emplace(
        definition.name, std::make_unique<VulkanGraphicsPipeline>()
    );

    TRY_CATCH(
        cachedPipeline->second->create(_device, descriptor),
        _graphicsPipelines.erase(cachedPipeline)
    );

    const VulkanGraphicsPipeline* pipelinePtr = cachedPipeline->second.get();

    return Expected(pipelinePtr);
}
