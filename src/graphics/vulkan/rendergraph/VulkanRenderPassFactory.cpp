#include "VulkanRenderPassFactory.h"

#include "graphics/vulkan/common/VulkanDebugger.h"

#include "VulkanRenderGraphBuilder.h"

#include "passes/CompositePass.h"
#include "passes/DebugPass.h"
#include "passes/MeshRenderPass.h"

void VulkanRenderPassFactory::registerPassTypes() {
    _factories[VulkanRenderPassType::MeshRender] = &createPassFactory<MeshRenderPass, MeshRenderPassCreateContext>;
    _factories[VulkanRenderPassType::Debug]      = &createPassFactory<DebugPass,      DebugPassCreateContext>;
    _factories[VulkanRenderPassType::Composite]  = &createPassFactory<CompositePass,  CompositePassCreateContext>;
}

Expected<std::unique_ptr<VulkanRenderPass>> VulkanRenderPassFactory::createPass(
    const VulkanRenderPassDescriptor&      descriptor,
    const VulkanRenderGraphBuilderContext& context
) const {
    const auto it = _factories.find(descriptor.type);

    if (it == _factories.end()) {
        return VK_FAIL("Failed to create render pass: no factory registered for pass type.");
    }

    return Expected(it->second(descriptor, context));
}
