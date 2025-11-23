#pragma once
#ifndef NOBLEENGINE_VULKANRENDERRESOURCES_H
#define NOBLEENGINE_VULKANRENDERRESOURCES_H

#include "nodes/VulkanRenderPassResource.h"

#include <memory>
#include <unordered_map>
#include <vector>

#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/pipeline/VulkanGraphicsPipeline.h"
#include "graphics/vulkan/pipeline/VulkanShaderProgram.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"

class VulkanRenderResources {
public:
    using ResourcesMap = std::unordered_map<std::string, std::unique_ptr<VulkanRenderPassResource>>;

    VulkanRenderResources()  = default;
    ~VulkanRenderResources() = default;

    VulkanRenderResources(const VulkanRenderResources&)            = delete;
    VulkanRenderResources& operator=(const VulkanRenderResources&) = delete;

    VulkanRenderResources(VulkanRenderResources&&)            = delete;
    VulkanRenderResources& operator=(VulkanRenderResources&&) = delete;

    [[nodiscard]] bool create(const vk::Device& device, uint32_t framesInFlight, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    [[nodiscard]] const ResourcesMap& getResources() const noexcept {
        return _resources;
    }

    void addResource(const VulkanRenderPassResource& resource) {
        _resources[resource.name] = std::make_unique<VulkanRenderPassResource>(resource);
    }

    [[nodiscard]] bool allocateDescriptors(
        VulkanPipelineDescriptor&                       pipelineDescriptor,
        const VulkanShaderProgram::DescriptorSchemeMap& descriptorSchemes,
        std::string&                                    errorMessage
    );

    static std::vector<vk::DescriptorSet> buildDescriptorSetsForFrame(
        const VulkanPipelineDescriptor& pipelineDescriptor, uint32_t currentFrameIndex
    );

private:
    vk::Device _device{};

    uint32_t _framesInFlight = 0;

    ResourcesMap _resources{};

    std::vector<std::unique_ptr<VulkanDescriptorManager>> _descriptorManagers{};
};

#endif // NOBLEENGINE_VULKANRENDERRESOURCES_H
