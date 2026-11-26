#pragma once

#include "core/debug/ErrorHandling.h"
#include "core/render/DrawList.h"

#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"

#include <functional>
#include <string>
#include <vector>

class VulkanRecordContext;

// Immutable, describes the pass identity
struct VulkanPassDescriptor {
    std::string name = "Undefined_Pass";

    std::vector<VulkanPassResourceUse> resources{};

    std::vector<std::string> pipelines{};

    std::function<Expected<void>(VulkanRecordContext&)> record;

    explicit VulkanPassDescriptor(std::string _name) : name(std::move(_name)) {}

    VulkanPassDescriptor& resource(const VulkanPassResourceUse& use) {
        resources.emplace_back(use);
        return *this;
    }

    VulkanPassDescriptor& pipeline(std::string _name) {
        pipelines.push_back(std::move(_name));
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
