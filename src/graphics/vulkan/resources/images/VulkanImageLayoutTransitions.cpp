#include "VulkanImageLayoutTransitions.h"

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <string>

namespace VulkanImageLayoutTransitions {
    std::optional<LayoutTransition> getLayoutTransition(
        const vk::ImageLayout oldLayout, const vk::ImageLayout newLayout
    ) {
        using ImageLayout = vk::ImageLayout;

        // Supported layout transitions
        if (oldLayout == ImageLayout::eUndefined &&
            newLayout == ImageLayout::eColorAttachmentOptimal)
            return LayoutTransition{
                {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput
            };

        if (oldLayout == ImageLayout::eUndefined &&
            newLayout == ImageLayout::eDepthStencilAttachmentOptimal)
            return LayoutTransition{
                {}, vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eEarlyFragmentTests
            };

        if (oldLayout == ImageLayout::eDepthStencilAttachmentOptimal &&
            newLayout == ImageLayout::eShaderReadOnlyOptimal)
            return LayoutTransition{
                vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::AccessFlagBits2::eShaderRead,
                vk::PipelineStageFlagBits2::eEarlyFragmentTests, vk::PipelineStageFlagBits2::eFragmentShader
            };

        if (oldLayout == ImageLayout::eShaderReadOnlyOptimal &&
            newLayout == ImageLayout::eDepthStencilAttachmentOptimal)
            return LayoutTransition{
                vk::AccessFlagBits2::eShaderRead,
                vk::AccessFlagBits2::eDepthStencilAttachmentRead | vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
                vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eEarlyFragmentTests
            };

        if (oldLayout == ImageLayout::eUndefined &&
            newLayout == ImageLayout::eTransferDstOptimal)
            return LayoutTransition{
                {}, vk::AccessFlagBits2::eTransferWrite,
                vk::PipelineStageFlagBits2::eTopOfPipe, vk::PipelineStageFlagBits2::eTransfer
            };

        if (oldLayout == ImageLayout::eTransferDstOptimal &&
            newLayout == ImageLayout::eTransferSrcOptimal) {
            return LayoutTransition{
                vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eTransferRead,
                vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eTransfer
            };
        }

        if (oldLayout == ImageLayout::eTransferSrcOptimal &&
            newLayout == ImageLayout::eShaderReadOnlyOptimal) {
            return LayoutTransition{
                vk::AccessFlagBits2::eTransferRead, vk::AccessFlagBits2::eShaderRead,
                vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader
            };
        }

        if (oldLayout == ImageLayout::eTransferDstOptimal &&
            newLayout == ImageLayout::eShaderReadOnlyOptimal)
            return LayoutTransition{
                vk::AccessFlagBits2::eTransferWrite, vk::AccessFlagBits2::eShaderRead,
                vk::PipelineStageFlagBits2::eTransfer, vk::PipelineStageFlagBits2::eFragmentShader
            };

        if (oldLayout == ImageLayout::eColorAttachmentOptimal &&
            newLayout == ImageLayout::eShaderReadOnlyOptimal) {
            return LayoutTransition{
                vk::AccessFlagBits2::eColorAttachmentWrite, vk::AccessFlagBits2::eShaderRead,
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eFragmentShader
            };
        }

        if (oldLayout == ImageLayout::eShaderReadOnlyOptimal &&
            newLayout == ImageLayout::eColorAttachmentOptimal) {
            return LayoutTransition{
                vk::AccessFlagBits2::eShaderRead, vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eFragmentShader, vk::PipelineStageFlagBits2::eColorAttachmentOutput
            };
        }

        if (oldLayout == ImageLayout::eColorAttachmentOptimal &&
            newLayout == ImageLayout::ePresentSrcKHR)
            return LayoutTransition{
                vk::AccessFlagBits2::eColorAttachmentWrite, {},
                vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::PipelineStageFlagBits2::eBottomOfPipe
            };

        if (oldLayout == ImageLayout::ePresentSrcKHR &&
            newLayout == ImageLayout::eColorAttachmentOptimal)
            return LayoutTransition{
                {}, vk::AccessFlagBits2::eColorAttachmentWrite,
                vk::PipelineStageFlagBits2::eBottomOfPipe, vk::PipelineStageFlagBits2::eColorAttachmentOutput
            };

        return std::nullopt;
    }

    bool transitionImageLayout(
        const vk::CommandBuffer commandBuffer,
        std::string&            errorMessage,
        const vk::Image         image,
        const vk::Format        format,
        const vk::ImageLayout   oldLayout,
        const vk::ImageLayout   newLayout,
        const uint32_t          mipLevels
    ) {
        if (oldLayout == newLayout) return true;

        // Specify which region of the image to transition
        vk::ImageSubresourceRange subresourceRange{};
        subresourceRange
            .setBaseMipLevel(0)
            .setLevelCount(mipLevels)
            .setBaseArrayLayer(0)
            .setLayerCount(1);

        if (VulkanImage::isDepthBuffer(format)) {
            subresourceRange.aspectMask = vk::ImageAspectFlagBits::eDepth;

            if (VulkanImage::hasStencilComponent(format)) {
                subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
            }
        } else {
            subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        }

        // Define how the transition operates and what to change
        const auto transition = getLayoutTransition(oldLayout, newLayout);

        if (!transition) {
            errorMessage = "Failed to transition Vulkan image layout: unsupported transition.";
            return false;
        }

        vk::ImageMemoryBarrier2 barrier{};
        barrier
            .setSrcStageMask(transition->srcStage)
            .setSrcAccessMask(transition->srcAccessMask)
            .setDstStageMask(transition->dstStage)
            .setDstAccessMask(transition->dstAccessMask)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setSrcQueueFamilyIndex(vk::QueueFamilyIgnored) // No ownership transfer to another queue family
            .setDstQueueFamilyIndex(vk::QueueFamilyIgnored)
            .setImage(image)
            .setSubresourceRange(subresourceRange);

        vk::DependencyInfo dependencyInfo{};
        dependencyInfo.setImageMemoryBarriers({barrier});

        commandBuffer.pipelineBarrier2(dependencyInfo);

        return true;
    }
}
