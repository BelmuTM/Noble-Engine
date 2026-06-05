#pragma once

#include "graphics/vulkan/pipeline/VulkanPipelineDescriptor.h"

#include "graphics/vulkan/rendergraph/draw/VulkanDrawCall.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderPassAttachment.h"

#include <memory>
#include <ranges>
#include <utility>

class VulkanGraphicsPipeline;

struct VulkanPassTransition {
    VulkanRenderPassResource* resource     = nullptr;
    vk::ImageLayout           targetLayout = vk::ImageLayout::eUndefined;

    bool operator==(const VulkanPassTransition& other) const {
        return resource == other.resource && targetLayout == other.targetLayout;
    }
};

enum class VulkanRenderPassType : std::uint8_t { None, MeshRender, Composite, Debug };

enum class VulkanRenderPassCullMode : std::uint8_t { None, Frustum };

// Immutable, describes the pass identity
struct VulkanRenderPassDescriptor {
    std::string name = "Undefined_Pass";

    std::string programPath;

    VulkanRenderPassType     type     = VulkanRenderPassType::None;
    VulkanRenderPassCullMode cullMode = VulkanRenderPassCullMode::None;

    std::vector<VulkanRenderPassAttachmentDescriptor> readDescriptors{};
    std::vector<VulkanRenderPassAttachmentDescriptor> writeDescriptors{};

    VulkanRenderPassAttachmentDescriptor depthAttachmentDescriptor{};
};

// Mutable, describes the pass state (data subject to hot reloading)
class VulkanRenderPass {
public:
    using DepthAttachment   = std::unique_ptr<VulkanRenderPassAttachment>;
    using AttachmentsVector = std::vector<std::unique_ptr<VulkanRenderPassAttachment>>;
    using TransitionsVector = std::vector<VulkanPassTransition>;

    using DescriptorSetsMap     = std::unordered_map<std::uint32_t, VulkanDescriptorSets*>;
    using DescriptorManagersMap = std::unordered_map<std::uint32_t, std::unique_ptr<VulkanDescriptorManager>>;

    using DrawCallsVector = std::vector<VulkanDrawCall>;

    explicit VulkanRenderPass(VulkanRenderPassDescriptor passDescriptor) : _passDescriptor(std::move(passDescriptor)) {}

    ~VulkanRenderPass() = default;

    VulkanRenderPass(const VulkanRenderPass&)            = delete;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    VulkanRenderPass(VulkanRenderPass&&)            = delete;
    VulkanRenderPass& operator=(VulkanRenderPass&&) = delete;

    void destroy() noexcept;

    // Getters

    [[nodiscard]] const VulkanRenderPassDescriptor& getPassDescriptor() const noexcept { return _passDescriptor; }

    [[nodiscard]]       VulkanShaderProgram*& getShaderProgram()       noexcept { return _shaderProgram; }
    [[nodiscard]] const VulkanShaderProgram*  getShaderProgram() const noexcept { return _shaderProgram; }

    [[nodiscard]]       VulkanPipelineDescriptor& getPipelineDescriptor()       noexcept { return _pipelineDescriptor; }
    [[nodiscard]] const VulkanPipelineDescriptor& getPipelineDescriptor() const noexcept { return _pipelineDescriptor; }

    [[nodiscard]] const VulkanGraphicsPipeline* getPipeline() const noexcept { return _pipeline; }

    [[nodiscard]]       DescriptorSetsMap& getDescriptorSets()       noexcept { return _descriptorSets; }
    [[nodiscard]] const DescriptorSetsMap& getDescriptorSets() const noexcept { return _descriptorSets; }

    [[nodiscard]]       DescriptorManagersMap& getDescriptorManagers()       noexcept { return _descriptorManagers; }
    [[nodiscard]] const DescriptorManagersMap& getDescriptorManagers() const noexcept { return _descriptorManagers; }

    [[nodiscard]] VulkanRenderPassAttachment* getDepthAttachment() const noexcept { return _depthAttachment.get(); }

    [[nodiscard]]       AttachmentsVector& getColorAttachments()       noexcept { return _colorAttachments; }
    [[nodiscard]] const AttachmentsVector& getColorAttachments() const noexcept { return _colorAttachments; }

    [[nodiscard]]       DrawCallsVector& getDrawCalls()       noexcept { return _drawCalls; }
    [[nodiscard]] const DrawCallsVector& getDrawCalls() const noexcept { return _drawCalls; }

    [[nodiscard]]       TransitionsVector& getEntryTransitions()       noexcept { return _entryTransitions; }
    [[nodiscard]] const TransitionsVector& getEntryTransitions() const noexcept { return _entryTransitions; }

    [[nodiscard]]       TransitionsVector& getExitTransitions()       noexcept { return _exitTransitions; }
    [[nodiscard]] const TransitionsVector& getExitTransitions() const noexcept { return _exitTransitions; }

    // Setters

    VulkanRenderPass& setPipelineDescriptor(const VulkanPipelineDescriptor& pipelineDescriptor) noexcept {
        _pipelineDescriptor = pipelineDescriptor;
        return *this;
    }

    VulkanRenderPass& setPipeline(const VulkanGraphicsPipeline* pipeline) noexcept {
        _pipeline = pipeline;
        return *this;
    }

    VulkanRenderPass& setDepthAttachment(std::unique_ptr<VulkanRenderPassAttachment> depthAttachment) noexcept {
        _depthAttachment = std::move(depthAttachment);
        return *this;
    }

    VulkanRenderPass& setDepthAttachment(const VulkanRenderPassAttachment& depthAttachment) noexcept {
        _depthAttachment = std::make_unique<VulkanRenderPassAttachment>(depthAttachment);
        return *this;
    }

    VulkanRenderPass& addColorAttachment(const VulkanRenderPassAttachment& colorAttachment) noexcept {
        _colorAttachments.push_back(std::make_unique<VulkanRenderPassAttachment>(colorAttachment));
        return *this;
    }

    VulkanDrawCall& emplaceDrawCall() noexcept {
        return _drawCalls.emplace_back();
    }

    VulkanRenderPass& addEntryTransition(const VulkanPassTransition& entryTransition) {
        const auto cachedTransition = std::ranges::find(_entryTransitions, entryTransition);
        if (cachedTransition == _entryTransitions.end()) {
            _entryTransitions.push_back(entryTransition);
        }
        return *this;
    }

    VulkanRenderPass& addExitTransition(const VulkanPassTransition& exitTransition) {
        const auto cachedTransition = std::ranges::find(_exitTransitions, exitTransition);
        if (cachedTransition == _exitTransitions.end()) {
            _exitTransitions.push_back(exitTransition);
        }
        return *this;
    }

private:
    const VulkanRenderPassDescriptor _passDescriptor;

    VulkanShaderProgram* _shaderProgram = nullptr;

    VulkanPipelineDescriptor      _pipelineDescriptor{};
    const VulkanGraphicsPipeline* _pipeline  = nullptr;

    DepthAttachment   _depthAttachment{};
    AttachmentsVector _colorAttachments{};

    DescriptorSetsMap     _descriptorSets{};
    DescriptorManagersMap _descriptorManagers{};

    DrawCallsVector _drawCalls{};

    TransitionsVector _entryTransitions{};
    TransitionsVector _exitTransitions{};
};
