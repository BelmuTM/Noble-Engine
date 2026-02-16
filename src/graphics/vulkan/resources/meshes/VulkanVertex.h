#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "core/resources/models/Vertex.h"

namespace VulkanVertex {
    static vk::VertexInputBindingDescription getBindingDescription() noexcept {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static constexpr std::array<vk::VertexInputAttributeDescription, 5> getAttributeDescriptions() noexcept {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat   , offsetof(Vertex, position     )},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat   , offsetof(Vertex, normal       )},
            vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, tangent      )},
            vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32B32Sfloat   , offsetof(Vertex, color        )},
            vk::VertexInputAttributeDescription{4, 0, vk::Format::eR32G32Sfloat      , offsetof(Vertex, textureCoords)}
        };
    }
}
