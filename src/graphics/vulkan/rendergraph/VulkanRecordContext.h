#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanFrameResources;
class VulkanFrameCuller;
class VulkanRenderObjectManager;
class VulkanDescriptorSets;
class VulkanMaterial;

class VulkanRecordContext {
public:
    VulkanRecordContext(
        const vk::CommandBuffer                  commandBuffer,
        const vk::Extent2D                       extent,
        const std::uint32_t                      frameIndex,
        const VulkanFrameResources*              frame,
        const VulkanRenderObjectManager*         renderObjectManager,
        const VulkanFrameCuller*                 frameCuller,
        const vk::detail::DispatchLoaderDynamic& dispatchLoader
    )
        :
        _commandBuffer(commandBuffer),
        _extent(extent),
        _frameIndex(frameIndex),
        _frame(frame),
        _renderObjectManager(renderObjectManager),
        _frameCuller(frameCuller),
        _dispatchLoader(dispatchLoader)
    {}

    VulkanRecordContext(const VulkanRecordContext&)            = delete;
    VulkanRecordContext& operator=(const VulkanRecordContext&) = delete;

    VulkanRecordContext(VulkanRecordContext&&)            = delete;
    VulkanRecordContext& operator=(VulkanRecordContext&&) = delete;

    [[nodiscard]] vk::CommandBuffer commandBuffer() const noexcept { return _commandBuffer; }

    [[nodiscard]] vk::Extent2D extent() const noexcept { return _extent; }

    [[nodiscard]] std::uint32_t frameIndex() const noexcept { return _frameIndex; }

    [[nodiscard]] const vk::detail::DispatchLoaderDynamic& dispatchLoader() const noexcept { return _dispatchLoader; }

    void bindGlobalDescriptors(
        vk::PipelineLayout    pipelineLayout,
        vk::PipelineBindPoint pipelineBindPoint
    ) const;


    void bindPassDescriptors(
        vk::PipelineLayout          pipelineLayout,
        vk::PipelineBindPoint       pipelineBindPoint,
        const VulkanDescriptorSets* sets
    ) const;


    void bindMaterial(
        vk::PipelineLayout    pipelineLayout,
        vk::PipelineBindPoint pipelineBindPoint,
        const VulkanMaterial* material
    ) const;

private:
    vk::CommandBuffer _commandBuffer;

    vk::Extent2D  _extent;
    std::uint32_t _frameIndex;

    const VulkanFrameResources*      _frame;
    const VulkanRenderObjectManager* _renderObjectManager;
    const VulkanFrameCuller*         _frameCuller;

    const vk::detail::DispatchLoaderDynamic& _dispatchLoader;
};
