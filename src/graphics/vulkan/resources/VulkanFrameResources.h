#pragma once

#include "descriptors/VulkanDescriptorManager.h"
#include "descriptors/VulkanDescriptorSets.h"

#include "images/VulkanImageManager.h"

#include "ubo/VulkanFrameUniformBuffer.h"
#include "ubo/VulkanUniformBufferManager.h"

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
        const VulkanImageManager&   imageManager,
        VulkanUniformBufferManager& uniformBufferManager,
        uint32_t                    framesInFlight,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    void update(uint32_t frameIndex, uint32_t imageIndex, const FrameUniforms& uniforms);

    [[nodiscard]] uint32_t getFrameIndex() const noexcept { return _frameIndex; }
    [[nodiscard]] uint32_t getImageIndex() const noexcept { return _imageIndex; }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptors() const noexcept { return _frameUBODescriptors; }

private:
    const VulkanDevice*       _device       = nullptr;
    const VulkanImageManager* _imageManager = nullptr;

    uint32_t _framesInFlight = 0;
    uint32_t _frameIndex     = 0;
    uint32_t _imageIndex     = 0;

    VulkanDescriptorManager _descriptorManager{};

    VulkanFrameUniformBuffer    _frameUBO{};
    VulkanDescriptorSets* _frameUBODescriptors = nullptr;
};
