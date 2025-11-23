#include "VulkanRenderResources.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanRenderResources::create(
    const vk::Device& device, const uint32_t framesInFlight, std::string& errorMessage
) noexcept {
    _device         = device;
    _framesInFlight = framesInFlight;

    return true;
}

void VulkanRenderResources::destroy() noexcept {
    for (const auto& descriptorManager : _descriptorManagers) {
        descriptorManager->destroy();
    }

    _descriptorManagers.clear();

    _device = nullptr;
}

bool VulkanRenderResources::allocateDescriptors(
    VulkanPipelineDescriptor&                       pipelineDescriptor,
    const VulkanShaderProgram::DescriptorSchemeMap& descriptorSchemes,
    std::string&                                    errorMessage
) {
    pipelineDescriptor.descriptorSetGroups.clear();

    for (const auto& scheme : descriptorSchemes | std::views::values) {
        auto descriptorManager = std::make_unique<VulkanDescriptorManager>();

        TRY(descriptorManager->create(_device, scheme, _framesInFlight, 1, errorMessage));

        pipelineDescriptor.descriptorLayouts.push_back(descriptorManager->getLayout());

        VulkanDescriptorSets* descriptorSets = nullptr;
        TRY(descriptorManager->allocate(descriptorSets, errorMessage));

        for (const auto& descriptor : scheme) {
            if (auto it = _resources.find(descriptor.name); it != _resources.end()) {
                const auto& resource = it->second;

                if (resource->imageObject) {
                    descriptorSets->bindPerFrameResource(resource->imageObject->getDescriptorInfo(descriptor.binding));
                }
            }
        }

        pipelineDescriptor.descriptorSetGroups.push_back(descriptorSets);

        _descriptorManagers.push_back(std::move(descriptorManager));
    }

    return true;
}

std::vector<vk::DescriptorSet> VulkanRenderResources::buildDescriptorSetsForFrame(
    const VulkanPipelineDescriptor& pipelineDescriptor, const uint32_t currentFrameIndex
) {
    std::vector<vk::DescriptorSet> sets{};
    sets.reserve(pipelineDescriptor.descriptorSetGroups.size());

    for (const auto& group : pipelineDescriptor.descriptorSetGroups) {
        if (!group) {
            sets.push_back(VK_NULL_HANDLE);
            continue;
        }
        const auto groupSets = group->getSets();
        if (currentFrameIndex >= groupSets.size()) {
            sets.push_back(VK_NULL_HANDLE);
            continue;
        }
        sets.push_back(groupSets[currentFrameIndex]);
    }

    return sets;
}
