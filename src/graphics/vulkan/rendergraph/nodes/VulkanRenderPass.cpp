#include "VulkanRenderPass.h"

void VulkanRenderPass::destroy() noexcept {
    for (const auto& descriptorManager : _descriptorManagers | std::views::values) {
        descriptorManager->destroy();
    }
}
