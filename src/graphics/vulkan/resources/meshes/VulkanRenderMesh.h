#pragma once

#include "core/render/DrawFlags.h"

class VulkanMesh;
class VulkanMaterial;

struct VulkanRenderMesh {
    VulkanMesh*     mesh     = nullptr;
    VulkanMaterial* material = nullptr;

    DrawFlags drawFlags = DrawFlags::Bit::Opaque;
};
