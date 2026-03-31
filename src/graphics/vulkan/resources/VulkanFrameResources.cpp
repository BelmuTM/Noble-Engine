#include "VulkanFrameResources.h"

#include "images/VulkanImageLayoutTransitions.h"

#include "core/debug/ErrorHandling.h"

bool VulkanFrameResources::create(
    const VulkanDevice&         device,
    const VulkanImageManager&   imageManager,
    VulkanUniformBufferManager& uniformBufferManager,
    const uint32_t              framesInFlight,
    std::string&                errorMessage
) noexcept {
    _device         = &device;
    _imageManager   = &imageManager;
    _framesInFlight = framesInFlight;

    // Descriptors creation
    TRY_deprecated(_descriptorManager.create(device.getLogicalDevice(), frameDescriptorScheme, framesInFlight, 1, errorMessage));

    TRY_deprecated(uniformBufferManager.createBuffer(_frameUBO, errorMessage));

    TRY_deprecated(_descriptorManager.allocate(_frameUBODescriptors, errorMessage));
    _frameUBODescriptors->bindPerFrameUBO(_frameUBO, 0);

    return true;
}

void VulkanFrameResources::destroy() noexcept {
    _descriptorManager.destroy();

    _device       = nullptr;
    _imageManager = nullptr;
}

void VulkanFrameResources::update(
    const uint32_t frameIndex, const uint32_t imageIndex, const FrameUniforms& uniforms
) {
    _frameUBO.update(frameIndex, uniforms);

    _frameIndex = frameIndex;
    _imageIndex = imageIndex;
}
