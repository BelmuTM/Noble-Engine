#pragma once

#include "core/debug/ErrorHandling.h"
#include "core/render/DrawList.h"

#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"

#include <functional>
#include <string>
#include <vector>

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

class VulkanRecordContext;

// Immutable, describes the pass identity
struct VulkanPassDescriptor {
    std::string name = "Undefined_Pass";

    std::vector<VulkanPassResourceUse> resources{};

    std::vector<VulkanPipelineId> pipelines{};

    std::function<Expected<void>(VulkanRecordContext&)> record;

    explicit VulkanPassDescriptor(std::string _name) : name(std::move(_name)) {}

    VulkanPassDescriptor& resource(const VulkanPassResourceUse& use) {
        resources.emplace_back(use);
        return *this;
    }

    VulkanPassDescriptor& pipeline(const VulkanPipelineId id) {
        pipelines.push_back(id);
        return *this;
    }

    VulkanPassDescriptor& execute(const std::function<Expected<void>(VulkanRecordContext&)>& recordFunction) {
        record = recordFunction;
        return *this;
    }

    [[nodiscard]] std::vector<VulkanGraphResourceId> getSampledResources() const {
        std::vector<VulkanGraphResourceId> resourceIds{};

        for (const auto& use : resources) {
            if (use.usage == VulkanGraphResourceUsage::SampledRead) {
                resourceIds.push_back(use.resourceId);
            }
        }

        return resourceIds;
    }
};
