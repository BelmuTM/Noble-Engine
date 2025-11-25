#include "VulkanRenderResources.h"

#include <iostream>

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
    _descriptorBindingsPerManager.clear();
    _descriptorSetGroups.clear();

    _device = nullptr;
}

[[nodiscard]] bool VulkanRenderResources::recreate(std::string& errorMessage) {
    if (_descriptorSetGroups.empty() || _descriptorManagers.empty()) {
        return true;
    }

    for (size_t managerIndex = 0; managerIndex < _descriptorManagers.size(); managerIndex++) {
        const VulkanDescriptorSets* descriptorSets = _descriptorSetGroups[managerIndex];

        if (!descriptorSets) continue;

        const auto& bindings = _descriptorBindingsPerManager[managerIndex];

        for (const auto& [binding, resourceName] : bindings) {
            auto it = _resources.find(resourceName);
            if (it == _resources.end()) continue;

            const auto& resource = it->second;
            if (!resource->image) continue;

            descriptorSets->bindPerFrameResource(resource->image->getDescriptorInfo(binding));
        }
    }

    return true;
}

bool VulkanRenderResources::createColorAttachments(
    VulkanRenderPass*         pass,
    const VulkanImageManager& imageManager,
    VulkanFrameResources&     frameResources,
    std::string&              errorMessage
) {
    for (const auto& colorOutput : pass->getShaderProgram()->getStageOutputs()) {
        VulkanImage* colorImage = frameResources.allocateColorBuffer();

        constexpr auto format = vk::Format::eB8G8R8A8Srgb;

        TRY(imageManager.createColorBuffer(
            *colorImage,
            format,
            frameResources.getFrameContext().extent,
            errorMessage
        ));

        VulkanRenderPassResource colorBuffer{};
        colorBuffer
            .setName(colorOutput)
            .setType(Buffer)
            .setImage(colorImage)
            .setFormat(format)
            .setLayout(vk::ImageLayout::eColorAttachmentOptimal);

        colorBuffer.currentLayout = vk::ImageLayout::eColorAttachmentOptimal;

        VulkanRenderPassAttachment colorAttachment{};
        colorAttachment
            .setResource(colorBuffer)
            .setLoadOp(vk::AttachmentLoadOp::eClear)
            .setStoreOp(vk::AttachmentStoreOp::eStore)
            .setClearValue(vk::ClearColorValue{0.0f, 0.0f, 0.0f, 1.0f});

        pass->addColorAttachment(colorAttachment);

        addResource(colorBuffer);

        _resourceWriters[colorOutput].push_back(pass);
    }

    return true;
}

bool VulkanRenderResources::allocateDescriptors(VulkanRenderPass* pass, std::string& errorMessage) {
    _descriptorManagers.clear();
    _descriptorBindingsPerManager.clear();
    _descriptorSetGroups.clear();

    for (const auto& scheme : pass->getShaderProgram()->getDescriptorSchemes() | std::views::values) {
        auto descriptorManager = std::make_unique<VulkanDescriptorManager>();

        TRY(descriptorManager->create(_device, scheme, _framesInFlight, 1, errorMessage));

        VulkanDescriptorSets* descriptorSets = nullptr;
        TRY(descriptorManager->allocate(descriptorSets, errorMessage));

        _descriptorSetGroups.push_back(descriptorSets);
        _descriptorBindingsPerManager.emplace_back();

        const size_t managerIndex = _descriptorSetGroups.size() - 1;

        for (const auto& descriptor : scheme) {
            if (auto it = _resources.find(descriptor.name); it != _resources.end()) {
                const auto& resource = it->second;

                if (resource->image) {
                    descriptorSets->bindPerFrameResource(resource->image->getDescriptorInfo(descriptor.binding));
                }

                _descriptorBindingsPerManager[managerIndex].push_back({descriptor.binding, descriptor.name});

                _resourceReaders[descriptor.name].push_back(pass);
            }
        }

        pass->getPipelineDescriptor().descriptorLayouts.push_back(descriptorManager->getLayout());

        _descriptorManagers.push_back(std::move(descriptorManager));
    }

    return true;
}

std::vector<vk::DescriptorSet> VulkanRenderResources::buildDescriptorSets(const uint32_t currentFrameIndex) const {
    std::vector<vk::DescriptorSet> sets{};
    sets.reserve(_descriptorSetGroups.size());

    for (const auto& group : _descriptorSetGroups) {
        const auto& groupSets = group->getSets();
        if (currentFrameIndex >= groupSets.size()) {
            sets.emplace_back(VK_NULL_HANDLE);
            continue;
        }
        sets.push_back(groupSets[currentFrameIndex]);
    }

    return sets;
}
