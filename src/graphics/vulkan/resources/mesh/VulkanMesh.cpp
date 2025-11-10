#include "VulkanMesh.h"

VulkanMesh VulkanMesh::makeFullscreenTriangle() {
    VulkanMesh fullscreenTriangle{};

    static const std::vector<Vertex> vertices = {
        {{-1.0f, -1.0f, 0.0f}, {}, {}, {0.0f, 0.0f}},
        {{ 3.0f, -1.0f, 0.0f}, {}, {}, {2.0f, 0.0f}},
        {{-1.0f,  3.0f, 0.0f}, {}, {}, {0.0f, 2.0f}},
    };

    static const std::vector<uint16_t> indices = {0, 1, 2};

    fullscreenTriangle.loadData(vertices, indices);
    fullscreenTriangle.setBufferless(false);

    return fullscreenTriangle;
}
