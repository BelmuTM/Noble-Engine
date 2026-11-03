#pragma once

#include "core/render/BindingSlots.h"

#include "graphics/vulkan/common/VulkanHeader.h"

class VulkanDrawBatchBuilder;

struct VulkanFrameContext {
    vk::Extent2D extent{};

    std::uint32_t frameIndex = 0;

    std::array<vk::DescriptorSet, BindingSlots::PassData> globalSets{};

    vk::Buffer vertexBuffer{};
    vk::Buffer indexBuffer{};

    vk::QueryPool queryPool{};

    const vk::detail::DispatchLoaderDynamic* dispatchLoader = nullptr;

    const VulkanDrawBatchBuilder* drawBatchBuilder = nullptr;
};
