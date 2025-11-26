#include "VulkanFrameResources.h"

#include "images/VulkanImageLayoutTransitions.h"

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

    // Descriptors creation
    TRY(_descriptorManager.create(device.getLogicalDevice(), frameDescriptorScheme, framesInFlight, 1, errorMessage));

    TRY(uniformBufferManager.createBuffer(_frameUBO, errorMessage));

    TRY(_descriptorManager.allocate(_frameUBODescriptors, errorMessage));
    _frameUBODescriptors->bindPerFrameUBO(_frameUBO, 0);

    return true;
}

void VulkanFrameResources::destroy() noexcept {
    _descriptorManager.destroy();

    for (const auto& colorBuffer : _colorBuffers) {
        colorBuffer->destroy(*_device);
    }

    _device       = nullptr;
    _swapchain    = nullptr;
    _imageManager = nullptr;
}

void VulkanFrameResources::update(
    const uint32_t frameIndex, const uint32_t imageIndex, const Camera& camera
) {
    _frameUBO.update(frameIndex, *_swapchain, camera);

    _frameIndex = frameIndex;
    _imageIndex = imageIndex;
}

bool VulkanFrameResources::recreate(const VulkanCommandManager* commandManager, std::string& errorMessage) const {
    for (auto& colorBuffer : _colorBuffers) {
        colorBuffer->destroy(*_device);

        TRY(_imageManager->createColorBuffer(
            *colorBuffer, colorBuffer->getFormat(), _swapchain->getExtent(), errorMessage
        ));

        TRY(colorBuffer->transitionLayout(
            commandManager, errorMessage,
            colorBuffer->getLayout(),
            vk::ImageLayout::eShaderReadOnlyOptimal
        ));
    }

    return true;
}

VulkanImage* VulkanFrameResources::allocateColorBuffer() {
    _colorBuffers.push_back(std::make_unique<VulkanImage>());
    return _colorBuffers.back().get();
}
