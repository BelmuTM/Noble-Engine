#include "VulkanRenderPass.h"

void VulkanRenderPass::destroy() const noexcept {
    if (_descriptorManager) {
        _descriptorManager->destroy();
    }
}
