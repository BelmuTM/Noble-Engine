#pragma once
#ifndef NOBLEENGINE_VULKANFRAMERESOURCES_H
#define NOBLEENGINE_VULKANFRAMERESOURCES_H

#include "descriptors/VulkanDescriptorManager.h"
#include "descriptors/VulkanDescriptorSets.h"
#include "images/VulkanImageManager.h"
#include "ubo/VulkanUniformBufferManager.h"
#include "ubo/FrameUniformBuffer.h"

static const VulkanDescriptorManager::DescriptorScheme frameDescriptorScheme = {
    {vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics}
};

class VulkanFrameResources {
public:
    VulkanFrameResources()  = default;
    ~VulkanFrameResources() = default;

    VulkanFrameResources(const VulkanFrameResources&)            = delete;
    VulkanFrameResources& operator=(const VulkanFrameResources&) = delete;

    VulkanFrameResources(VulkanFrameResources&&)            = delete;
    VulkanFrameResources& operator=(VulkanFrameResources&&) = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&         device,
        const VulkanSwapchain&      swapchain,
        const VulkanImageManager&   imageManager,
        VulkanUniformBufferManager& uniformBufferManager,
        uint32_t                    framesInFlight,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    void update(uint32_t frameIndex, const Camera& camera) const;

    [[nodiscard]] bool recreate(std::string& errorMessage);

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const std::unique_ptr<VulkanDescriptorSets>& getUBODescriptors() const noexcept {
        return _frameUBODescriptors;
    }

    [[nodiscard]] const VulkanImage& getDepthBuffer() const noexcept { return _depthBuffer; }

private:
    const VulkanDevice*       _device       = nullptr;
    const VulkanSwapchain*    _swapchain    = nullptr;
    const VulkanImageManager* _imageManager = nullptr;

    uint32_t _framesInFlight = 0;

    VulkanDescriptorManager _descriptorManager{};

    FrameUniformBuffer                    _frameUBO{};
    std::unique_ptr<VulkanDescriptorSets> _frameUBODescriptors{};

    VulkanImage _depthBuffer{};
};

#endif // NOBLEENGINE_VULKANFRAMERESOURCES_H
