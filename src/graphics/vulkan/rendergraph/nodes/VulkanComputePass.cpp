#include "VulkanComputePass.h"

void VulkanComputePass::destroy() const noexcept {
    _pass.destroy();
}
