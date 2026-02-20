#include "VulkanRenderPassFactory.h"

#include "VulkanRenderGraphBuilder.h"
#include "core/debug/Logger.h"

#include "passes/CompositePass.h"
#include "passes/DebugPass.h"
#include "passes/MeshRenderPass.h"

void VulkanRenderPassFactory::registerPassTypes() {
    _factories[VulkanRenderPassType::MeshRender] = &createPassFactory<MeshRenderPass, MeshRenderPassCreateContext>;
    _factories[VulkanRenderPassType::Debug]      = &createPassFactory<DebugPass,      DebugPassCreateContext>;
    _factories[VulkanRenderPassType::Composite]  = &createPassFactory<CompositePass,  CompositePassCreateContext>;
}

std::unique_ptr<VulkanRenderPass> VulkanRenderPassFactory::createPass(
    const std::string&                     path,
    const VulkanRenderPassType             type,
    const VulkanRenderGraphBuilderContext& context,
    std::string&                           errorMessage
) const {
    const auto it = _factories.find(type);
    if (it == _factories.end()) {
        errorMessage = "Failed to create Vulkan render pass: no factory registered for pass type.";
        return nullptr;
    }

    return it->second(path, context, errorMessage);
}
