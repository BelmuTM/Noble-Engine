#include "VulkanRenderPass.h"

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
