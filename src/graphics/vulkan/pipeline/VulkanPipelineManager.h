#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/compute/VulkanComputePipeline.h"
#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipeline.h"

#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgram.h"
#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgramManager.h"

#include <memory>
#include <unordered_map>
#include <variant>
#include <vector>

class VulkanPipelineManager {
public:
    using PipelineDefinition = std::variant<VulkanGraphicsPipelineDefinition, VulkanComputePipelineDefinition>;

    template<typename PipelineType>
    using PipelinesMap = std::unordered_map<std::size_t, std::unique_ptr<PipelineType>>;

    VulkanPipelineManager()  = default;
    ~VulkanPipelineManager() = default;

    VulkanPipelineManager(const VulkanPipelineManager&)            = delete;
    VulkanPipelineManager& operator=(const VulkanPipelineManager&) = delete;

    VulkanPipelineManager(VulkanPipelineManager&&)            = delete;
    VulkanPipelineManager& operator=(VulkanPipelineManager&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device) noexcept;

    void destroy() noexcept;

    [[nodiscard]] VulkanPipelineId registerPipeline(const PipelineDefinition& definition) {
        _definitions.push_back(definition);
        _shaderPrograms.push_back(nullptr);
        return static_cast<VulkanPipelineId>(_definitions.size() - 1);
    }

    [[nodiscard]] Expected<void> loadShaderPrograms(VulkanShaderProgramManager& shaderProgramManager);

    [[nodiscard]] Expected<const VulkanGraphicsPipeline*> getOrCreate(
        VulkanPipelineId                    id,
        const VulkanGraphicsPipelineTarget& target
    );

    [[nodiscard]] const PipelineDefinition& getPipelineDefinition(const VulkanPipelineId id) const noexcept {
        return _definitions[id];
    }

    [[nodiscard]] const VulkanShaderProgram& getProgram(const VulkanPipelineId id) const noexcept {
        assert(_shaderPrograms[id] && "loadPrograms() was not called");
        return *_shaderPrograms[id];
    }

    [[nodiscard]] const PipelinesMap<VulkanGraphicsPipeline>& getGraphicsPipelines() const noexcept {
        return _graphicsPipelines;
    }

    [[nodiscard]] const PipelinesMap<VulkanComputePipeline>& getComputePipelines() const noexcept {
        return _computePipelines;
    }

private:
    vk::Device _device{};

    std::vector<PipelineDefinition> _definitions{};

    std::vector<const VulkanShaderProgram*> _shaderPrograms{};

    PipelinesMap<VulkanGraphicsPipeline> _graphicsPipelines{};
    PipelinesMap<VulkanComputePipeline>  _computePipelines{};
};
