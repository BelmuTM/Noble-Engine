#include "VulkanRenderPassFactory.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "VulkanRenderGraphBuilder.h"

#include "nodes/VulkanCompositePass.h"
#include "nodes/VulkanDebugPass.h"
#include "nodes/VulkanMeshRenderPass.h"

void VulkanRenderPassFactory::registerPassTypes() {
    _factories[VulkanGraphicsPassType::MeshRender] =
        &createPassFactory<VulkanMeshRenderPass, VulkanMeshRenderPassCreateContext>;

    _factories[VulkanGraphicsPassType::Debug] =
        &createPassFactory<VulkanDebugPass, VulkanDebugPassCreateContext>;

    _factories[VulkanGraphicsPassType::Composite] =
        &createPassFactory<VulkanCompositePass, VulkanCompositePassCreateContext>;
}

Expected<void> VulkanRenderPassFactory::createPass(
    VulkanGraphicsPass* pass, const VulkanRenderGraphBuilderContext& context
) const {
    const auto passFactory = _factories.find(pass->getGraphicsPassDescriptor().type);

    if (passFactory == _factories.end()) {
        return VK_FAIL("Failed to create render pass: no factory registered for pass type.");
    }

    TRY(Expected(passFactory->second(pass, context)));

    return {};
}
