#include "DebugPass.h"

#include "core/debug/ErrorHandling.h"

bool DebugPass::create(
    const std::string&            path,
    const DebugPassCreateContext& context,
    std::string&                  errorMessage
) {
    TRY(context.shaderProgramManager.load(getShaderProgram(), path, errorMessage));

    const std::string& passName = std::filesystem::path(path).stem().string();

    const VulkanPipelineDescriptor pipelineDescriptor{
        getShaderProgram(),
        {
            context.frameResources.getDescriptorManager().getLayout(),
            context.renderObjectManager.getDescriptorManager().getLayout()
        }
    };

    setType(VulkanRenderPassType::Composite);
    setName(passName + "_DebugPass");
    setPipelineDescriptor(pipelineDescriptor);
    setBindPoint(vk::PipelineBindPoint::eGraphics);

    for (const auto& renderObject : context.renderObjectManager.getRenderObjects()) {
        auto aabbDraw = std::make_unique<VulkanDrawCall>();
    }

    return true;
}
