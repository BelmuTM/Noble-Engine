#include "VulkanRenderPassFactory.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "VulkanRenderGraphBuilder.h"

#include "nodes/VulkanCompositePass.h"
#include "nodes/VulkanDebugPass.h"
#include "nodes/VulkanMeshRenderPass.h"

void VulkanRenderPassFactory::registerPassTypes() {
    _factories[VulkanRenderPassType::MeshRender] =
        &createPassFactory<VulkanMeshRenderPass, VulkanMeshRenderPassCreateContext>;

    _factories[VulkanRenderPassType::Debug] =
        &createPassFactory<VulkanDebugPass, VulkanDebugPassCreateContext>;

    _factories[VulkanRenderPassType::Composite] =
        &createPassFactory<VulkanCompositePass, VulkanCompositePassCreateContext>;
}

Expected<std::unique_ptr<VulkanRenderPass>> VulkanRenderPassFactory::createPass(
    const VulkanRenderPassDescriptor&      descriptor,
    const VulkanRenderGraphBuilderContext& context
) const {
    const auto passFactory = _factories.find(descriptor.type);

    if (passFactory == _factories.end()) {
        return VK_FAIL("Failed to create render pass: no factory registered for pass type.");
    }

    return Expected(passFactory->second(descriptor, context));
}
