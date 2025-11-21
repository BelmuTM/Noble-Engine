#pragma once
#ifndef NOBLEENGINE_VULKANRENDERRESOURCES_H
#define NOBLEENGINE_VULKANRENDERRESOURCES_H

#include "nodes/VulkanRenderPassResource.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"

class VulkanRenderResources {
public:
    VulkanRenderResources()  = default;
    ~VulkanRenderResources() = default;

    VulkanRenderResources(const VulkanRenderResources&)            = delete;
    VulkanRenderResources& operator=(const VulkanRenderResources&) = delete;

    VulkanRenderResources(VulkanRenderResources&&)            = delete;
    VulkanRenderResources& operator=(VulkanRenderResources&&) = delete;

    void addResource(const VulkanRenderPassResource& resource) {
        _resources.emplace(resource.name, std::make_unique<VulkanRenderPassResource>(resource));
    }

    [[nodiscard]] bool allocateDescriptors(
        std::unordered_map<uint32_t, VulkanDescriptorScheme> descriptorSchemes, std::string& errorMessage
    );

private:
    std::unordered_map<std::string, std::unique_ptr<VulkanRenderPassResource>> _resources{};

    std::vector<std::shared_ptr<VulkanDescriptorManager>> _descriptorManagers{};
    std::vector<VulkanDescriptorSets*>   _descriptors{};
};

#endif // NOBLEENGINE_VULKANRENDERRESOURCES_H
