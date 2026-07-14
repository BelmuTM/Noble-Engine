#include "VulkanFrameResources.h"

#include "graphics/vulkan/resources/images/VulkanImageLayoutTransitions.h"

Expected<void> VulkanFrameResources::create(
    const VulkanDevice&         device,
    const VulkanImageManager&   imageManager,
    VulkanUniformBufferManager& uniformBufferManager,
    const std::uint32_t         framesInFlight
) noexcept {
    _device         = &device;
    _imageManager   = &imageManager;
    _framesInFlight = framesInFlight;

    // Descriptors creation
    TRY(_descriptorManager.create(device.getLogicalDevice(), getDescriptorScheme(), framesInFlight, 1));

    TRY_ASSIGN(_frameUBO, uniformBufferManager.allocateBuffer<VulkanFrameUniformBuffer>());

    TRY(_descriptorManager.allocate(_frameUBODescriptors));

    _frameUBODescriptors->updatePerFrameUBODescriptorSets(*_frameUBO, 0);

    return {};
}

void VulkanFrameResources::destroy() noexcept {
    _descriptorManager.destroy();

    _device       = nullptr;
    _imageManager = nullptr;
}

void VulkanFrameResources::update(
    const std::uint32_t frameIndex, const std::uint32_t imageIndex, const FrameUniforms& uniforms
) {
    _frameUBO->update(frameIndex, uniforms);

    _frameIndex = frameIndex;
    _imageIndex = imageIndex;
}
