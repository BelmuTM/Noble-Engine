#include "VulkanCompositePass.h"

Expected<void> VulkanCompositePass::create(const VulkanCompositePassCreateContext& context) {

    getPipelineLayoutDescriptor().descriptorLayouts = {
        context.frameResources.getDescriptorManager().getLayout()
    };

    emplaceDrawCall().setRenderMesh({
        context.meshManager.allocateMesh(VulkanMesh::makeFullscreenTriangle())
    });

    return {};
}
