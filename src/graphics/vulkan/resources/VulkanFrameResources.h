#pragma once
#ifndef NOBLEENGINE_VULKANFRAMERESOURCES_H
#define NOBLEENGINE_VULKANFRAMERESOURCES_H

#include "descriptors/VulkanDescriptorManager.h"
#include "descriptors/VulkanDescriptorSets.h"
#include "images/VulkanImageManager.h"
#include "ubo/VulkanUniformBufferManager.h"
#include "ubo/FrameUniformBuffer.h"

static const VulkanDescriptorScheme frameDescriptorScheme = {
    {0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics}
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

    void update(uint32_t frameIndex, uint32_t imageIndex, const Camera& camera);

    [[nodiscard]] uint32_t getFrameIndex() const noexcept { return _frameIndex; }
    [[nodiscard]] uint32_t getImageIndex() const noexcept { return _imageIndex; }

    [[nodiscard]] vk::Extent2D getExtent() const { return _swapchain->getExtent(); }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptors() const noexcept { return _frameUBODescriptors; }

    [[nodiscard]] const FrameUniforms& getUniforms() const noexcept { return _frameUBO.getUniforms(); }

private:
    const VulkanDevice*       _device       = nullptr;
    const VulkanSwapchain*    _swapchain    = nullptr;
    const VulkanImageManager* _imageManager = nullptr;

    uint32_t _framesInFlight = 0;
    uint32_t _frameIndex     = 0;
    uint32_t _imageIndex     = 0;

    VulkanDescriptorManager _descriptorManager{};

    FrameUniformBuffer    _frameUBO{};
    VulkanDescriptorSets* _frameUBODescriptors = nullptr;
};

#endif // NOBLEENGINE_VULKANFRAMERESOURCES_H
