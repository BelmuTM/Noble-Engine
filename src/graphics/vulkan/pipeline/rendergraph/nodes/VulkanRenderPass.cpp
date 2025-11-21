#include "VulkanRenderPass.h"

#include "core/debug/Logger.h"
#include "graphics/vulkan/resources/VulkanFrameResources.h"

VulkanRenderPass& VulkanRenderPass::addObjectDrawCall(const VulkanRenderObject* renderObject) {
    // Each submesh requires its own draw call
    for (const auto& submesh : renderObject->submeshes) {
        auto verticesDraw = std::make_unique<VulkanDrawCallWithPushConstants>();
        verticesDraw->setMesh(submesh.mesh);

        verticesDraw->setDescriptorResolver(
            [&submesh](const VulkanFrameContext& frame) {
                return std::vector{submesh.descriptorSets->getSets().at(frame.frameIndex)};
            }
        );

        verticesDraw->setPushConstant("object", &renderObject->data);

        addDrawCall(std::move(verticesDraw));
    }

    return *this;
}

bool VulkanRenderPass::createColorAttachments(
    const VulkanShaderProgram& shaderProgram,
    const VulkanImageManager&  imageManager,
    VulkanFrameResources&      frameResources,
    VulkanRenderResources&     renderResources,
    std::string&               errorMessage
) {
    for (const auto& colorOutput : shaderProgram.getStageOutputs()) {
        VulkanImage* colorImage = frameResources.allocateColorBuffer();

        size_t bufferIndex = frameResources.getColorBuffers().size() - 1;

        constexpr auto format = vk::Format::eB8G8R8A8Srgb;

        TRY(imageManager.createColorBuffer(
            *colorImage,
            format,
            frameResources.getFrameContext().extent,
            errorMessage
        ));

        VulkanRenderPassResource colorBuffer{};
        colorBuffer
            .setName(colorOutput)
            .setType(Buffer)
            .setImage(*colorImage)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal)
            .setFormat(format)
            .setResolveImageView([bufferIndex, &frameResources](const VulkanFrameContext&) {
                return frameResources.getColorBuffers()[bufferIndex]->getImageView();
            });

        VulkanRenderPassAttachment colorAttachment{};
        colorAttachment
            .setResource(colorBuffer)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});

        addColorAttachment(colorAttachment);
        renderResources.addResource(colorBuffer);

        Logger::debug("Added color attachment: " + colorOutput);
    }

    return true;
}
