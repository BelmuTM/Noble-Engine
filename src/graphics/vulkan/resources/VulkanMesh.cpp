#include "VulkanMesh.h"

VulkanMesh::VulkanMesh(const VulkanMesh& other) noexcept {
    _device       = other._device;
    _vertexOffset = other._vertexOffset;
    _indexOffset  = other._indexOffset;
}

VulkanMesh& VulkanMesh::operator=(const VulkanMesh& other) noexcept {
    if (this != &other) {
        destroy();

        _device       = other._device;
        _vertexOffset = other._vertexOffset;
        _indexOffset  = other._indexOffset;
    }
    return *this;
}

bool VulkanMesh::create(
    const VulkanDevice& device,
    std::string&        errorMessage
) noexcept {
    _device = &device;
    return true;
}

void VulkanMesh::destroy() noexcept {
    _device = nullptr;

    _vertexOffset = 0;
    _indexOffset  = 0;
}

void VulkanMesh::load() {
    // TO-DO
}
