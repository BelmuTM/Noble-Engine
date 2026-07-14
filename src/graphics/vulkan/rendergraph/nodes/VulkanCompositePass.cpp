#include "VulkanCompositePass.h"

Expected<void> VulkanCompositePass::create(const VulkanCompositePassCreateContext& context) {

    emplaceDrawCall().setRenderMesh({
        context.meshManager.allocateMesh(VulkanMesh::makeFullscreenTriangle())
    });

    return {};
}
