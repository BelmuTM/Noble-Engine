#pragma once
#ifndef NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
#define NOBLEENGINE_VULKANGRAPHICSPIPELINE_H

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanShaderProgram.h"
#include "rendergraph/nodes/VulkanRenderPassAttachment.h"

struct VulkanPipelineDescriptor {
    const VulkanShaderProgram* shaderProgram = nullptr;

    std::vector<vk::DescriptorSetLayout> descriptorLayouts{};
};

class VulkanGraphicsPipeline {
public:
    VulkanGraphicsPipeline()  = default;
    ~VulkanGraphicsPipeline() = default;

    // Implicit conversion operator
    operator vk::Pipeline() const { return _pipeline; }

    VulkanGraphicsPipeline(const VulkanGraphicsPipeline&)            = delete;
    VulkanGraphicsPipeline& operator=(const VulkanGraphicsPipeline&) = delete;

    VulkanGraphicsPipeline(VulkanGraphicsPipeline&&)            = delete;
    VulkanGraphicsPipeline& operator=(VulkanGraphicsPipeline&&) = delete;

    [[nodiscard]] bool create(
        const vk::Device&               device,
        const VulkanPipelineDescriptor& descriptor,
        const AttachmentsVector&        colorAttachments,
        std::string&                    errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] const vk::PipelineLayout& getLayout() const noexcept { return _pipelineLayout; }

    [[nodiscard]] vk::ShaderStageFlags getStageFlags() const noexcept { return _stageFlags; }

private:
    vk::Device _device{};

    vk::Pipeline       _pipeline{};
    vk::PipelineLayout _pipelineLayout{};

    vk::ShaderStageFlags _stageFlags{};

    /*---------------------------------------*/
    /*        Pipeline State Structs         */
    /*---------------------------------------*/

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

    static constexpr vk::PipelineMultisampleStateCreateInfo multisamplingInfo = []{
        vk::PipelineMultisampleStateCreateInfo info{};
        info
            .setRasterizationSamples(vk::SampleCountFlagBits::e1)
            .setSampleShadingEnable(vk::False);
        return info;
    }();

    static constexpr vk::PipelineDepthStencilStateCreateInfo depthStencilInfo = []{
        vk::PipelineDepthStencilStateCreateInfo info{};
        info
            .setDepthTestEnable(vk::True)
            .setDepthWriteEnable(vk::True)
            .setDepthCompareOp(vk::CompareOp::eLessOrEqual)
            .setDepthBoundsTestEnable(vk::False)
            .setStencilTestEnable(vk::False);
        return info;
    }();

    static constexpr vk::PipelineColorBlendAttachmentState colorBlendAttachment = []{
        vk::PipelineColorBlendAttachmentState info{};
        info
            .setColorWriteMask(
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
            )
            .setBlendEnable(vk::False)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero)
            .setAlphaBlendOp(vk::BlendOp::eAdd);
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

    [[nodiscard]] bool createPipelineLayout(
        const vk::Device&                           device,
        const std::vector<vk::DescriptorSetLayout>& descriptorSetLayouts,
        const PushConstantsMap&                     pushConstantRanges,
        std::string&                                errorMessage
    );

    [[nodiscard]] bool createPipeline(
        const vk::Device&          device,
        const VulkanShaderProgram& shaderProgram,
        const AttachmentsVector&   colorAttachments,
        std::string&               errorMessage
    );
};

#endif //NOBLEENGINE_VULKANGRAPHICSPIPELINE_H
