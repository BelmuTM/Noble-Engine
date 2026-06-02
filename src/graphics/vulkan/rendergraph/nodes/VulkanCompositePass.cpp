#include "VulkanCompositePass.h"

Expected<void> VulkanCompositePass::create(const VulkanCompositePassCreateContext& context) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), getPassDescriptor().programPath));

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {context.frameResources.getDescriptorManager().getLayout()}
    };

    setPipelineDescriptor(pipelineDescriptor);

    VulkanMesh* fullscreenMesh = context.meshManager.allocateMesh(VulkanMesh::makeFullscreenTriangle());

    emplaceDrawCall().setRenderMesh({fullscreenMesh});

    return {};
}
