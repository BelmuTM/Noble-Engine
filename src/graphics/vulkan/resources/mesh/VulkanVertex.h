#pragma once
#ifndef NOBLEENGINE_VULKANVERTEX_H
#define NOBLEENGINE_VULKANVERTEX_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "core/objects/object/Vertex.h"

namespace VulkanVertex {
    static vk::VertexInputBindingDescription getBindingDescription() {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 4> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, position     )},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, normal       )},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color        )},
            vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32Sfloat   , offsetof(Vertex, textureCoords)}
        };
    }
}

#endif // NOBLEENGINE_VULKANVERTEX_H
