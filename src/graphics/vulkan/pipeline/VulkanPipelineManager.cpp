#include "VulkanPipelineManager.h"

#include "common/HashUtils.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include <ranges>

Expected<void> VulkanPipelineManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanPipelineManager::destroy() noexcept {

    _definitions.clear();

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

Expected<void> VulkanPipelineManager::loadShaderPrograms(VulkanShaderProgramManager& shaderProgramManager) {

    _shaderPrograms.resize(_definitions.size(), nullptr);

    for (VulkanPipelineId id = 0; id < _definitions.size(); id++) {

        const std::string& path = std::visit(
            [](const auto& definition) -> const std::string& { return definition.programPath; },
            _definitions[id]
        );

        Expected<const VulkanShaderProgram*> shaderProgram = shaderProgramManager.load(path);

        if (shaderProgram.failed()) {
            return VK_FAIL("Failed to load program \"" + path + "\" for pipeline " + std::to_string(id));
        }

        _shaderPrograms[id] = shaderProgram.value();
    }

    return {};
}

namespace {

[[nodiscard]] std::size_t hashGraphicsPipeline(
    const VulkanPipelineId id, const VulkanGraphicsPipelineTarget& target
) {
    std::size_t seed = 0;

    HashUtils::combine(seed, id);

    for (const vk::Format format : target.colorAttachmentFormats) {
        HashUtils::combine(seed, static_cast<std::uint32_t>(format));
    }

    HashUtils::combine(seed, static_cast<std::uint32_t>(target.depthAttachmentFormat));
    HashUtils::combine(seed, target.depthWrite);

    for (const vk::DescriptorSetLayout layout : target.layout.descriptorLayouts) {
        HashUtils::combine(seed, static_cast<VkDescriptorSetLayout>(layout));
    }

    return seed;
}

}

Expected<const VulkanGraphicsPipeline*> VulkanPipelineManager::getOrCreate(
    const VulkanPipelineId              id,
    const VulkanGraphicsPipelineTarget& target
) {

    const std::size_t pipelineHash = hashGraphicsPipeline(id, target);

    const auto pipelineIt = _graphicsPipelines.find(pipelineHash);

    if (pipelineIt != _graphicsPipelines.end()) {
        const VulkanGraphicsPipeline* pipelinePtr = pipelineIt->second.get();
        return Expected(pipelinePtr);
    }

    const VulkanGraphicsPipelineDefinition* definition =
        std::get_if<VulkanGraphicsPipelineDefinition>(&_definitions[id]);

    if (!definition) {
        return VK_FAIL("Failed to create graphics pipeline: provided pipeline ID is compute.");
    }

    auto [layout, colorAttachmentFormats, depthAttachmentFormat, depthWrite] = target;

    const VulkanShaderProgram* shaderProgram = _shaderPrograms[id];

    layout.pushConstantRanges = shaderProgram->getPushConstants();

    const VulkanGraphicsPipelineDescriptor descriptor(
        shaderProgram->getStages(),
        definition->state,
        layout,
        colorAttachmentFormats,
        depthAttachmentFormat,
        depthWrite
    );

    auto [cachedPipeline, inserted] = _graphicsPipelines.emplace(
        pipelineHash, std::make_unique<VulkanGraphicsPipeline>()
    );

    TRY_CATCH(
        cachedPipeline->second->create(_device, descriptor),
        _graphicsPipelines.erase(cachedPipeline)
    );

    const VulkanGraphicsPipeline* pipelinePtr = cachedPipeline->second.get();

    return Expected(pipelinePtr);
}
