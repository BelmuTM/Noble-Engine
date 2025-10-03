#pragma once
#ifndef NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
#define NOBLEENGINE_VULKANGRAPHICSPIPELINE_H

#include "graphics/vulkan/common/VulkanHeader.h"
#include "graphics/vulkan/core/VulkanCommandManager.h"
#include "graphics/vulkan/core/VulkanDevice.h"
#include "graphics/vulkan/core/VulkanSwapchain.h"
#include "graphics/vulkan/core/memory/VulkanBuffer.h"

#include <glm/glm.hpp>

struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

static constexpr vk::DeviceSize uniformBufferSize = sizeof(UniformBufferObject);

class VulkanGraphicsPipeline {
public:
    VulkanGraphicsPipeline()  = default;
    ~VulkanGraphicsPipeline() = default;

    // Implicit conversion operator
    operator vk::Pipeline() const { return pipeline; }

    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&)            = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;
    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&)                 = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&)      = delete;

    [[nodiscard]] bool create(
        const VulkanDevice&    device,
        const VulkanSwapchain& swapchain,
        uint32_t               framesInFlight,
        std::string&           errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] const vk::PipelineLayout& getLayout() const { return pipelineLayout; }

    [[nodiscard]] const std::vector<VulkanBuffer>& getUniformBuffers() const { return uniformBuffers; }

    [[nodiscard]] const std::vector<vk::DescriptorSet>& getDescriptorSets() const { return descriptorSets; }

private:
    const VulkanDevice*    _device    = nullptr;
    const VulkanSwapchain* _swapchain = nullptr;

    vk::DescriptorSetLayout descriptorSetLayout{};

    vk::Pipeline       pipeline{};
    vk::PipelineLayout pipelineLayout{};

    std::vector<VulkanBuffer> uniformBuffers{};

    vk::DescriptorPool             descriptorPool{};
    std::vector<vk::DescriptorSet> descriptorSets{};

    // -------------------------------
    //       Vulkan Info structs
    // -------------------------------

    static constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo = []{
        vk::PipelineInputAssemblyStateCreateInfo info{};
        info
            .setTopology(vk::PrimitiveTopology::eTriangleList)
            .setPrimitiveRestartEnable(vk::False);
        return info;
    }();

    static constexpr vk::PipelineViewportStateCreateInfo viewportInfo = []{
        vk::PipelineViewportStateCreateInfo info{};
        info
            .setViewportCount(1)
            .setScissorCount(1)
            .setPViewports(nullptr)
            .setPScissors(nullptr);
        return info;
    }();

    static constexpr vk::PipelineRasterizationStateCreateInfo rasterizationInfo = []{
        vk::PipelineRasterizationStateCreateInfo info{};
        info
            .setDepthClampEnable(vk::False)
            .setRasterizerDiscardEnable(vk::False)
            .setPolygonMode(vk::PolygonMode::eFill)
            .setCullMode(vk::CullModeFlagBits::eBack)
            .setFrontFace(vk::FrontFace::eCounterClockwise)
            .setDepthBiasEnable(vk::False)
            .setLineWidth(1.0f);
        return info;
    }();

    static constexpr vk::PipelineMultisampleStateCreateInfo multisamplingInfo = []{
        vk::PipelineMultisampleStateCreateInfo info{};
        info
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False);
        return info;
    }();

    static constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment = []{
        vk::PipelineColorBlendAttachmentState info{};
        info
            .setColorWriteMask(
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            )
            .setBlendEnable(vk::True)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd);
        return info;
    }();

    static constexpr vk::PipelineColorBlendStateCreateInfo colorBlendInfo = []{
        vk::PipelineColorBlendStateCreateInfo info{};
        info
            .setLogicOpEnable(vk::False)
            .setLogicOp(vk::LogicOp::eCopy)
            .setAttachmentCount(1)
            .setPAttachments(&colorBlendAttachment);
        return info;
    }();

    static constexpr std::array<vk::DynamicState, 2> dynamicStates = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor
    };

    inline static const vk::PipelineDynamicStateCreateInfo dynamicStateInfo = []{
        vk::PipelineDynamicStateCreateInfo info{};
        info.setDynamicStates(dynamicStates);
        return info;
    }();

    bool createDescriptorSetLayout(std::string& errorMessage);

    bool createPipelineLayout(std::string& errorMessage);
    bool createPipeline(std::string& errorMessage);

    bool createUniformBuffers(uint32_t framesInFlight, std::string& errorMessage);

    bool createDescriptorPool(uint32_t framesInFlight, std::string& errorMessage);
    bool createDescriptorSets(uint32_t framesInFlight, std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
