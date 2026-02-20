#include "CompositePass.h"

#include "core/debug/ErrorHandling.h"

bool CompositePass::create(
    const std::string&                path,
    const CompositePassCreateContext& context,
    std::string&                      errorMessage
) {
    TRY_deprecated(context.shaderProgramManager.load(getShaderProgram(), path, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {context.frameResources.getDescriptorManager().getLayout()}
    };

    setType(VulkanRenderPassType::Composite);
    setName(passName + "_CompositePass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);

    const VulkanMesh* fullscreenMesh = context.meshManager.allocateMesh(VulkanMesh::makeFullscreenTriangle());

    auto fullscreenDraw = std::make_unique<VulkanDrawCall>();
    fullscreenDraw->setMesh(fullscreenMesh);
    addDrawCall(std::move(fullscreenDraw));

    return true;
}
