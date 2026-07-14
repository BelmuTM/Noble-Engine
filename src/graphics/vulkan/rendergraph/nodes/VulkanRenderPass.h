#pragma once

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/rendergraph/draw/VulkanDrawCall.h"
#include "graphics/vulkan/rendergraph/resources/VulkanRenderPassAttachment.h"

#include <memory>
#include <ranges>

class VulkanGraphicsPipeline;

struct VulkanPassTransition {
    VulkanRenderPassResource* resource     = nullptr;
    vk::ImageLayout           targetLayout = vk::ImageLayout::eUndefined;

    bool operator==(const VulkanPassTransition& other) const {
        return resource == other.resource && targetLayout == other.targetLayout;
    }
};

// TODO: Make graphics API agnostic
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

    using DrawCallsVector = std::vector<VulkanDrawCall>;

    explicit VulkanRenderPass(VulkanRenderPassDescriptor passDescriptor) : _passDescriptor(std::move(passDescriptor)) {}

    ~VulkanRenderPass() = default;

    VulkanRenderPass(const VulkanRenderPass&)            = delete;
    VulkanRenderPass& operator=(const VulkanRenderPass&) = delete;

    VulkanRenderPass(VulkanRenderPass&&)            = delete;
    VulkanRenderPass& operator=(VulkanRenderPass&&) = delete;

    void destroy() const noexcept;

    // Getters

    [[nodiscard]] const VulkanRenderPassDescriptor& getPassDescriptor() const noexcept { return _passDescriptor; }

    [[nodiscard]]       VulkanShaderProgram*& getShaderProgram()       noexcept { return _shaderProgram; }
    [[nodiscard]] const VulkanShaderProgram*  getShaderProgram() const noexcept { return _shaderProgram; }

    [[nodiscard]]       VulkanDescriptorManager* getDescriptorManager()       noexcept { return _descriptorManager.get(); }
    [[nodiscard]] const VulkanDescriptorManager* getDescriptorManager() const noexcept { return _descriptorManager.get(); }

    [[nodiscard]]       VulkanDescriptorSets* getDescriptorSets()       noexcept { return _descriptorSets; }
    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _descriptorSets; }

    [[nodiscard]] VulkanPipelineLayoutDescriptor& getPipelineLayoutDescriptor() noexcept {
        return _pipelineLayoutDescriptor;
    }
    [[nodiscard]] const VulkanPipelineLayoutDescriptor& getPipelineLayoutDescriptor() const noexcept {
        return _pipelineLayoutDescriptor;
    }

    [[nodiscard]] const VulkanGraphicsPipeline* getPipeline() const noexcept { return _pipeline; }

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

    VulkanRenderPass& setShaderProgram(VulkanShaderProgram* shaderProgram) noexcept {
        _shaderProgram = shaderProgram;
        return *this;
    }

    VulkanRenderPass& setDescriptorManager(std::unique_ptr<VulkanDescriptorManager> descriptorManager) noexcept {
        _descriptorManager = std::move(descriptorManager);
        return *this;
    }

    VulkanRenderPass& setDescriptorSets(VulkanDescriptorSets* descriptorSets) noexcept {
        _descriptorSets = descriptorSets;
        return *this;
    }

    VulkanRenderPass& setPipelineLayoutDescriptor(
        const VulkanPipelineLayoutDescriptor& pipelineLayoutDescriptor
    ) noexcept {
        _pipelineLayoutDescriptor = pipelineLayoutDescriptor;
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
        return _drawCalls.emplace_back()
            .bindPipelineLayoutDescriptor(&_pipelineLayoutDescriptor);
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

    std::unique_ptr<VulkanDescriptorManager> _descriptorManager{};
    VulkanDescriptorSets*                    _descriptorSets = nullptr;

    VulkanPipelineLayoutDescriptor _pipelineLayoutDescriptor{};
    const VulkanGraphicsPipeline*  _pipeline = nullptr;

    DepthAttachment   _depthAttachment{};
    AttachmentsVector _colorAttachments{};

    DrawCallsVector _drawCalls{};

    TransitionsVector _entryTransitions{};
    TransitionsVector _exitTransitions{};
};
