#pragma once

#include "core/render/DrawFlags.h"

#include "graphics/vulkan/rendergraph/nodes/VulkanPass.h"

#include "graphics/vulkan/pipeline/graphics/VulkanGraphicsPipelineState.h"

class VulkanGraphicsRecordContext;

enum class VulkanGraphicsPassCullMode : std::uint8_t { None, Frustum };

struct VulkanGraphicsPassDescriptor {
    VulkanPassDescriptor base;

    std::function<Expected<void>(VulkanGraphicsRecordContext&)> record;

    VulkanGraphicsPipelineState pipelineState{};

    VulkanGraphicsPassCullMode cullMode = VulkanGraphicsPassCullMode::None;

    VulkanGraphicsPassAttachmentDescriptor depthAttachmentDescriptor{};

    DrawFlags drawFlags{};

    bool countPrimitives = false;
};

class VulkanGraphicsPipeline;

class VulkanGraphicsPass {
public:
    using AttachmentPointer = std::unique_ptr<VulkanGraphicsPassAttachment>;
    using AttachmentsVector = std::vector<AttachmentPointer>;

    explicit VulkanGraphicsPass(const VulkanGraphicsPassDescriptor& descriptor)
        : _pass(descriptor.base), _graphicsPassDescriptor(descriptor) {}

    ~VulkanGraphicsPass() = default;

    VulkanGraphicsPass(const VulkanGraphicsPass&)            = delete;
    VulkanGraphicsPass& operator=(const VulkanGraphicsPass&) = delete;

    VulkanGraphicsPass(VulkanGraphicsPass&&)            = delete;
    VulkanGraphicsPass& operator=(VulkanGraphicsPass&&) = delete;

    void destroy() const noexcept;

    // Getters

    [[nodiscard]]       VulkanPass& base()       noexcept { return _pass; }
    [[nodiscard]] const VulkanPass& base() const noexcept { return _pass; }

    [[nodiscard]] const VulkanGraphicsPassDescriptor& getGraphicsPassDescriptor() const noexcept {
        return _graphicsPassDescriptor;
    }

    [[nodiscard]] const VulkanGraphicsPipeline* getGraphicsPipeline() const noexcept { return _graphicsPipeline; }

    [[nodiscard]] VulkanGraphicsPassAttachment* getDepthAttachment() const noexcept { return _depthAttachment.get(); }

    [[nodiscard]]       AttachmentsVector& getColorAttachments()       noexcept { return _colorAttachments; }
    [[nodiscard]] const AttachmentsVector& getColorAttachments() const noexcept { return _colorAttachments; }

    // Setters

    VulkanGraphicsPass& setGraphicsPipeline(const VulkanGraphicsPipeline* graphicsPipeline) noexcept {
        _graphicsPipeline = graphicsPipeline;
        return *this;
    }

    VulkanGraphicsPass& setDepthAttachment(std::unique_ptr<VulkanGraphicsPassAttachment> depthAttachment) noexcept {
        _depthAttachment = std::move(depthAttachment);
        return *this;
    }

    VulkanGraphicsPass& setDepthAttachment(const VulkanGraphicsPassAttachment& depthAttachment) noexcept {
        _depthAttachment = std::make_unique<VulkanGraphicsPassAttachment>(depthAttachment);
        return *this;
    }

    VulkanGraphicsPass& addColorAttachment(const VulkanGraphicsPassAttachment& colorAttachment) noexcept {
        _colorAttachments.push_back(std::make_unique<VulkanGraphicsPassAttachment>(colorAttachment));
        return *this;
    }

private:
    VulkanPass _pass;

    VulkanGraphicsPassDescriptor _graphicsPassDescriptor;

    const VulkanGraphicsPipeline*  _graphicsPipeline = nullptr;

    AttachmentPointer _depthAttachment{};
    AttachmentsVector _colorAttachments{};
};
