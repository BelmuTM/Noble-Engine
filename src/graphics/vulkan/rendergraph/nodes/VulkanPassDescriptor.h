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

    DrawListId drawListId = 0;

    std::function<Expected<void>(VulkanRecordContext&)> record;

    [[nodiscard]] std::vector<std::string> getSampledResourcesNames() const {
        std::vector<std::string> names{};

        for (const auto& use : resources) {
            if (use.usage == VulkanGraphResourceUsage::SampledRead) {
                names.push_back(use.name);
            }
        }

        return names;
    }
};
