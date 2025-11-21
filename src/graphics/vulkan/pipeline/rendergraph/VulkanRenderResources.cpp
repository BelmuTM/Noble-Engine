#include "VulkanRenderResources.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanRenderResources::allocateDescriptors(
    std::unordered_map<uint32_t, VulkanDescriptorScheme> descriptorSchemes, std::string& errorMessage
) {

    for (const auto& scheme : descriptorSchemes | std::views::values) {
        auto descriptorManager = std::make_shared<VulkanDescriptorManager>();

        TRY(descriptorManager->create(device.getLogicalDevice(), scheme, framesInFlight, 1, errorMessage));
    }
}

