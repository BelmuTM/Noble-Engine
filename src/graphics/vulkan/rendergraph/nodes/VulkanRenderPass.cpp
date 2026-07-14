#include "VulkanRenderPass.h"

void VulkanRenderPass::destroy() const noexcept {
    _descriptorManager->destroy();
}
