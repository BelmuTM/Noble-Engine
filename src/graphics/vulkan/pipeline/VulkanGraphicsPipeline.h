#pragma once
#ifndef NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
#define NOBLEENGINE_VULKANGRAPHICSPIPELINE_H

#include "graphics/vulkan/core/VulkanSwapchain.h"
#include "graphics/vulkan/common/VulkanHeader.h"

#include <glm/glm.hpp>

struct Vertex {
    glm::vec2 position;
    glm::vec3 color;

    static vk::VertexInputBindingDescription getBindingDescription() {
        return {0, sizeof(Vertex), vk::VertexInputRate::eVertex};
    }

    static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
        return {
            vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32Sfloat   , offsetof(Vertex, position)},
            vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, color   )}
        };
    }
};

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
        const VulkanDevice& device, const VulkanSwapchain& swapchain, std::string& errorMessage
    ) noexcept;
    void destroy() noexcept;

    [[nodiscard]] vk::Buffer getVertexBuffer() const { return vertexBuffer; }

private:
    const VulkanDevice*    _device    = nullptr;
    const VulkanSwapchain* _swapchain = nullptr;

    vk::Buffer       vertexBuffer{};
    vk::DeviceMemory vertexBufferMemory{};

    vk::Pipeline       pipeline{};
    vk::PipelineLayout pipelineLayout{};

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
            .setCullMode(vk::CullModeFlagBits::eNone)
            .setFrontFace(vk::FrontFace::eClockwise)
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

    bool createBuffer(
        vk::Buffer& buffer, vk::DeviceMemory& bufferMemory, vk::DeviceSize size, vk::BufferUsageFlags usage,
        vk::MemoryPropertyFlags properties, std::string& errorMessage
    ) const;

    bool createVertexBuffer(std::string& errorMessage);

    bool createPipelineLayout(std::string& errorMessage);
    bool createPipeline(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
