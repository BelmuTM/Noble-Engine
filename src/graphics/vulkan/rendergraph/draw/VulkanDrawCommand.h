#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include <vector>

class VulkanMaterial;

struct VulkanDrawCommand {
    const VulkanMaterial* material = nullptr;

    vk::DrawIndexedIndirectCommand command{};
};

using VulkanDrawCommandsVector = std::vector<VulkanDrawCommand>;
