#include "VulkanGraphicsPass.h"

void VulkanGraphicsPass::destroy() const noexcept {
    _pass.destroy();
}
