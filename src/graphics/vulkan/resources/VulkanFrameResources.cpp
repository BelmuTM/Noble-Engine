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

    _frameContext.setExtent(swapchain.getExtent());

    // Descriptors creation
    TRY(_descriptorManager.create(device.getLogicalDevice(), frameDescriptorScheme, framesInFlight, 1, errorMessage));

    TRY(uniformBufferManager.createBuffer(_frameUBO, errorMessage));

    TRY(_descriptorManager.allocate(_frameUBODescriptors, errorMessage));
    _frameUBODescriptors->bindPerFrameUBO(_frameUBO, 0);

    // Depth buffer creation
    _depthBuffer = std::make_unique<VulkanImage>();
    TRY(imageManager.createDepthBuffer(*_depthBuffer, swapchain.getExtent(), errorMessage));

    VulkanRenderPassResource depthBufferResource{};
    depthBufferResource
        .setType(Buffer)
        .setImage(*_depthBuffer)
        .setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal)
        .setResolveImageView([this](const VulkanFrameContext&) { return _depthBuffer->getImageView(); });

    _depthBufferAttachment
        .setResource(depthBufferResource)
        .setLoadOp(vk::AttachmentLoadOp::eClear)
        .setStoreOp(vk::AttachmentStoreOp::eStore)
        .setClearValue(vk::ClearDepthStencilValue{1.0f, 0});

    return true;
}

void VulkanFrameResources::destroy() noexcept {
    _descriptorManager.destroy();

    _depthBuffer->destroy(*_device);

    for (const auto& colorBuffer : _colorBuffers) {
        colorBuffer->destroy(*_device);
    }
}

void VulkanFrameResources::update(
    const uint32_t frameIndex, const uint32_t imageIndex, const Camera& camera
) {
    _frameUBO.update(frameIndex, *_swapchain, camera);

    _frameContext
        .setFrameIndex(frameIndex)
        .setSwapchainImageView(_swapchain->getImageViews()[imageIndex])
        .setExtent(_swapchain->getExtent());
}

bool VulkanFrameResources::recreate(std::string& errorMessage) const {
    _depthBuffer->destroy(*_device);

    TRY(_imageManager->createDepthBuffer(*_depthBuffer, _swapchain->getExtent(), errorMessage));

    for (auto& colorBuffer : _colorBuffers) {
        colorBuffer->destroy(*_device);

        TRY(_imageManager->createColorBuffer(
            *colorBuffer, colorBuffer->getFormat(), _swapchain->getExtent(), errorMessage
        ));
    }

    return true;
}

VulkanImage* VulkanFrameResources::allocateColorBuffer() {
    _colorBuffers.push_back(std::make_unique<VulkanImage>());
    return _colorBuffers.back().get();
}
