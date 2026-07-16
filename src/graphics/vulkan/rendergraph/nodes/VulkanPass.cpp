#include "VulkanPass.h"

void VulkanPass::destroy() const noexcept {
    if (_descriptorManager) {
        _descriptorManager->destroy();
    }
}
