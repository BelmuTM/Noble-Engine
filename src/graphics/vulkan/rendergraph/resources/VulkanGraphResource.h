#pragma once

#include "graphics/vulkan/resources/images/VulkanImage.h"

#include <functional>
#include <utility>

enum class VulkanGraphResourceType : std::uint8_t { Transient, External, SwapchainOutput };

enum class VulkanGraphResourceUsage : std::uint8_t {
    ColorAttachment,
    DepthAttachment,
    DepthReadOnly,
    SampledRead,
    StorageRead,
    StorageWrite,
    StorageReadWrite,
    TransferSrc,
    TransferDst,
    Present
};

struct VulkanAttachmentOps {
    vk::AttachmentLoadOp  loadOp  = vk::AttachmentLoadOp::eClear;
    vk::AttachmentStoreOp storeOp = vk::AttachmentStoreOp::eStore;
};

struct VulkanPassResourceUse {
    std::string                        name;
    VulkanGraphResourceUsage           usage = VulkanGraphResourceUsage::ColorAttachment;
    std::optional<VulkanAttachmentOps> attachment;

    [[nodiscard]] static constexpr bool isAttachmentUsage(const VulkanGraphResourceUsage u) noexcept {
        return u == VulkanGraphResourceUsage::ColorAttachment
            || u == VulkanGraphResourceUsage::DepthAttachment
            || u == VulkanGraphResourceUsage::DepthReadOnly;
    }

    [[nodiscard]] static constexpr bool isDepthUsage(const VulkanGraphResourceUsage u) noexcept {
        return u == VulkanGraphResourceUsage::DepthAttachment
            || u == VulkanGraphResourceUsage::DepthReadOnly;
    }

    [[nodiscard]] static constexpr bool isWriteUsage(const VulkanGraphResourceUsage u) noexcept {
        using U = VulkanGraphResourceUsage;
        return u == U::ColorAttachment || u == U::DepthAttachment || u == U::StorageWrite
            || u == U::StorageReadWrite || u == U::TransferDst;
    }
};

struct VulkanGraphResourceDescriptor {
    std::string name = "Undefined_Resource";

    vk::Format format = vk::Format::eR8G8B8A8Unorm;

    VulkanGraphResourceType type = VulkanGraphResourceType::Transient;

    vk::ClearValue clearValue = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

    VulkanGraphResourceDescriptor& setName(const std::string& _name) noexcept {
        name = _name; return *this;
    }

    VulkanGraphResourceDescriptor& setFormat(const vk::Format _format) noexcept {
        format = _format; return *this;
    }

    VulkanGraphResourceDescriptor& setType(const VulkanGraphResourceType _type) noexcept {
        type = _type; return *this;
    }

    VulkanGraphResourceDescriptor& setClearValue(const vk::ClearValue _clearValue) noexcept {
        clearValue = _clearValue; return *this;
    }
};

struct VulkanGraphResource {
    VulkanGraphResourceDescriptor descriptor{};

    VulkanImage* image = nullptr;

    std::function<VulkanImage*()> imageResolver;

    explicit VulkanGraphResource(VulkanGraphResourceDescriptor _descriptor)
        : descriptor(std::move(_descriptor)) {}

    explicit operator bool() const noexcept {
        return static_cast<bool>(resolveImage());
    }

    [[nodiscard]] VulkanImage* resolveImage() const {
        if (image || !imageResolver) return image;
        return imageResolver();
    }

    VulkanGraphResource& setImage(VulkanImage* _image) noexcept { image = _image; return *this; }

    VulkanGraphResource& setImageResolver(std::function<VulkanImage*()> _imageResolver) noexcept {
        imageResolver = std::move(_imageResolver);
        return *this;
    }
};

struct VulkanGraphResourceState {
    vk::ImageLayout         layout = vk::ImageLayout::eUndefined;
    vk::PipelineStageFlags2 stages = vk::PipelineStageFlagBits2::eAllCommands;
    vk::AccessFlags2        access = vk::AccessFlagBits2::eMemoryRead | vk::AccessFlagBits2::eMemoryWrite;

    [[nodiscard]] static constexpr bool isWriteAccess(const vk::AccessFlags2 access) noexcept {
        using Access = vk::AccessFlagBits2;
        constexpr vk::AccessFlags2 writeMask =
            Access::eColorAttachmentWrite |
            Access::eDepthStencilAttachmentWrite |
            Access::eShaderStorageWrite |
            Access::eShaderWrite |
            Access::eTransferWrite;
        return static_cast<bool>(access & writeMask);
    }

    [[nodiscard]] bool needsBarrierTo(const VulkanGraphResourceState& nextState) const noexcept {
        return layout != nextState.layout || isWriteAccess(access) || isWriteAccess(nextState.access);
    }

    [[nodiscard]] static VulkanGraphResourceState stateForUsage(
        const VulkanGraphResourceUsage usage, const vk::PipelineStageFlags2 shaderStages
    ) {
        using Usage  = VulkanGraphResourceUsage;
        using Layout = vk::ImageLayout;
        using Stage  = vk::PipelineStageFlagBits2;
        using Access = vk::AccessFlagBits2;

        switch (usage) {

            case Usage::ColorAttachment:
                return {
                    Layout::eColorAttachmentOptimal,
                    Stage::eColorAttachmentOutput,
                    Access::eColorAttachmentRead | Access::eColorAttachmentWrite
                };

            case Usage::DepthAttachment:
                return {
                    Layout::eDepthStencilAttachmentOptimal,
                    Stage::eEarlyFragmentTests | Stage::eLateFragmentTests,
                    Access::eDepthStencilAttachmentRead | Access::eDepthStencilAttachmentWrite
                };

            case Usage::DepthReadOnly:
                return {
                    Layout::eDepthStencilReadOnlyOptimal,
                    Stage::eEarlyFragmentTests | Stage::eLateFragmentTests,
                    Access::eDepthStencilAttachmentRead
                };

            case Usage::SampledRead:
                return {Layout::eShaderReadOnlyOptimal, shaderStages, Access::eShaderSampledRead};

            case Usage::StorageRead:
                return {Layout::eGeneral, shaderStages, Access::eShaderStorageRead};

            case Usage::StorageWrite:
                return {Layout::eGeneral, shaderStages, Access::eShaderStorageWrite};

            case Usage::StorageReadWrite:
                return {Layout::eGeneral, shaderStages, Access::eShaderStorageRead | Access::eShaderStorageWrite};

            case Usage::TransferSrc:
                return {Layout::eTransferSrcOptimal, Stage::eAllTransfer, Access::eTransferRead};

            case Usage::TransferDst:
                return {Layout::eTransferDstOptimal, Stage::eAllTransfer, Access::eTransferWrite};

            case Usage::Present:
                return {Layout::ePresentSrcKHR, Stage::eAllCommands, {}};
        }

        return {};
    }
};
