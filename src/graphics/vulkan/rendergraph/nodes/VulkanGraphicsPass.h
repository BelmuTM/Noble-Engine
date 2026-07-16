#pragma once

#include "graphics/vulkan/rendergraph/nodes/VulkanPass.h"

#include "graphics/vulkan/rendergraph/draw/VulkanDrawCall.h"

// TODO: Make graphics API agnostic
enum class VulkanGraphicsPassType : std::uint8_t { None, MeshRender, Composite, Debug };

enum class VulkanGraphicsPassCullMode : std::uint8_t { None, Frustum };

struct VulkanGraphicsPassDescriptor {
    VulkanPassDescriptor base;

    VulkanGraphicsPassType     type     = VulkanGraphicsPassType::None;
    VulkanGraphicsPassCullMode cullMode = VulkanGraphicsPassCullMode::None;

    VulkanGraphicsPassAttachmentDescriptor depthAttachmentDescriptor{};
};

class VulkanGraphicsPipeline;

class VulkanGraphicsPass {
public:
    using AttachmentPointer = std::unique_ptr<VulkanGraphicsPassAttachment>;
    using AttachmentsVector = std::vector<AttachmentPointer>;

    using DrawCallsVector = std::vector<VulkanDrawCall>;

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

    [[nodiscard]]       DrawCallsVector& getDrawCalls()       noexcept { return _drawCalls; }
    [[nodiscard]] const DrawCallsVector& getDrawCalls() const noexcept { return _drawCalls; }

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

    VulkanDrawCall& emplaceDrawCall() noexcept {
        return _drawCalls.emplace_back()
            .bindPipelineLayoutDescriptor(&_pass.getPipelineLayoutDescriptor());
    }

private:
    VulkanPass _pass;

    VulkanGraphicsPassDescriptor _graphicsPassDescriptor;

    const VulkanGraphicsPipeline*  _graphicsPipeline = nullptr;

    AttachmentPointer _depthAttachment{};
    AttachmentsVector _colorAttachments{};

    DrawCallsVector _drawCalls{};
};
