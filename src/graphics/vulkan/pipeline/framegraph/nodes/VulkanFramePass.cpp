#include "VulkanFramePass.h"

VulkanFramePass& VulkanFramePass::addObjectDrawCall(const VulkanRenderObject* renderObject) {
    // Each submesh requires its own draw call
    for (const auto& submesh : renderObject->submeshes) {
        auto verticesDraw = std::make_unique<DrawCallPushConstant<ObjectDataGPU>>();
        verticesDraw->setMesh(*submesh.mesh);

        verticesDraw->setDescriptorResolver(
            [&submesh](const VulkanFrameContext& frame) {
                return std::vector{submesh.descriptorSets->getSets().at(frame.frameIndex)};
            }
        );

        verticesDraw->setPushConstantResolver([renderObject](const VulkanFrameContext&) {
            return renderObject->data;
        });

        addDrawCall(std::move(verticesDraw));
    }

    return *this;
}

/*
bool VulkanFramePass::createColorAttachments(
    const VulkanShaderProgram& shaderProgram,
    VulkanImageManager&        imageManager,
    const VulkanFrameContext&  frame,
    std::string&               errorMessage
) {
    for (const auto& colorOutput : shaderProgram.getStageOutputs()) {
        TRY(imageManager.createColorBuffer(
            compositeOutput,
            frame.extent,
            vk::Format::eR8G8B8A8Srgb,
            errorMessage
        ));

        VulkanFramePassResource compositeBuffer{};
        compositeBuffer
            .setType(Buffer)
            .setImage(compositeOutput)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setResolveImageView([this](const VulkanFrameContext&) { return compositeOutput.getImageView(); });

        VulkanFramePassAttachment compositeAttachment{};
        compositeAttachment
            .setResource(compositeBuffer)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});
    }
}
*/
