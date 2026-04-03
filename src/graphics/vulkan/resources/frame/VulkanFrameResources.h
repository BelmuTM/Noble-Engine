#pragma once

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include "graphics/vulkan/resources/images/VulkanImageManager.h"

#include "graphics/vulkan/resources/ubo/VulkanUniformBufferManager.h"

#include "VulkanFrameUniformBuffer.h"

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
        std::uint32_t               framesInFlight,
        std::string&                errorMessage
    ) noexcept;

    void destroy() noexcept;

    void update(std::uint32_t frameIndex, std::uint32_t imageIndex, const FrameUniforms& uniforms);

    [[nodiscard]] std::uint32_t getFrameIndex() const noexcept { return _frameIndex; }
    [[nodiscard]] std::uint32_t getImageIndex() const noexcept { return _imageIndex; }

    [[nodiscard]] static const VulkanDescriptorScheme& getFrameDescriptorScheme() noexcept {
        static const VulkanDescriptorScheme scheme = {
            {0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAllGraphics}
        };
        return scheme;
    }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptors() const noexcept { return _frameUBODescriptors; }

private:
    const VulkanDevice*       _device       = nullptr;
    const VulkanImageManager* _imageManager = nullptr;

    std::uint32_t _framesInFlight = 0;
    std::uint32_t _frameIndex     = 0;
    std::uint32_t _imageIndex     = 0;

    VulkanDescriptorManager _descriptorManager{};

    VulkanFrameUniformBuffer* _frameUBO            = nullptr;
    VulkanDescriptorSets*     _frameUBODescriptors = nullptr;
};
