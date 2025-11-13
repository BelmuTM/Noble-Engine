#include "VulkanFrameResources.h"

#include "core/debug/ErrorHandling.h"

bool VulkanFrameResources::create(
    const VulkanDevice&         device,
    const VulkanSwapchain&      swapchain,
    const VulkanImageManager&   imageManager,
    VulkanUniformBufferManager& uniformBufferManager,
    const uint32_t              framesInFlight,
    std::string&                errorMessage
) noexcept {
    _device         = &device;
    _swapchain      = &swapchain;
    _imageManager   = &imageManager;
    _framesInFlight = framesInFlight;

    // Descriptors
    TRY(_descriptorManager.create(device.getLogicalDevice(), frameDescriptorScheme, framesInFlight, 1, errorMessage));

    TRY(uniformBufferManager.createBuffer(_frameUBO, errorMessage));

    _frameUBODescriptors = std::make_unique<VulkanDescriptorSets>(_descriptorManager);
    TRY(_frameUBODescriptors->allocate(errorMessage));
    _frameUBODescriptors->bindPerFrameUBO(_frameUBO, 0);

    // Depth buffer
    TRY(imageManager.createDepthBuffer(_depthBuffer, swapchain.getExtent(), errorMessage));

    return true;
}

void VulkanFrameResources::destroy() noexcept {
    _descriptorManager.destroy();
    _depthBuffer.destroy(*_device);
}

void VulkanFrameResources::update(const uint32_t frameIndex, const Camera& camera) const {
    _frameUBO.update(frameIndex, *_swapchain, camera);
}

bool VulkanFrameResources::recreate(std::string& errorMessage) {
    _depthBuffer.destroy(*_device);

    TRY(_imageManager->createDepthBuffer(_depthBuffer, _swapchain->getExtent(), errorMessage));

    return true;
}
