#pragma once

#include "VulkanRenderPassAttachment.h"

#include "graphics/vulkan/pipeline/VulkanPipelineDescriptor.h"

#include "graphics/vulkan/rendergraph/draw/VulkanDrawCall.h"
#include "graphics/vulkan/resources/objects/VulkanRenderObject.h"

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
    std::string              name = "Undefined_Pass";
    std::string              programPath;
    VulkanRenderPassType     type     = VulkanRenderPassType::None;
    VulkanRenderPassCullMode cullMode = VulkanRenderPassCullMode::None;
};

// Mutable, describes the pass state (data subject to hot reloading)
class VulkanRenderPass {
public:
    using DescriptorSetsMap     = std::unordered_map<std::uint32_t, VulkanDescriptorSets*>;
    using DescriptorManagersMap = std::unordered_map<std::uint32_t, std::unique_ptr<VulkanDescriptorManager>>;

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

    [[nodiscard]] DescriptorManagersMap& getDescriptorManagers() noexcept { return _descriptorManagers; }

    [[nodiscard]] VulkanRenderPassAttachment* getDepthAttachment() const noexcept {
        return _depthAttachment.get();
    }

    [[nodiscard]] std::vector<std::unique_ptr<VulkanRenderPassAttachment>>& getColorAttachments() noexcept {
        return _colorAttachments;
    }

    [[nodiscard]] const std::vector<std::unique_ptr<VulkanRenderPassAttachment>>& getColorAttachments() const noexcept {
        return _colorAttachments;
    }

    [[nodiscard]] std::vector<VulkanDrawCall>& getDrawCalls() noexcept {
        return _drawCalls;
    }

    [[nodiscard]] const std::vector<VulkanDrawCall>& getDrawCalls() const noexcept {
        return _drawCalls;
    }

    [[nodiscard]]       std::vector<VulkanPassTransition>& getTransitions()       noexcept { return _transitions; }
    [[nodiscard]] const std::vector<VulkanPassTransition>& getTransitions() const noexcept { return _transitions; }

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

    VulkanRenderPass& setDepthAttachment(const VulkanRenderPassAttachment* prototype) noexcept {
        _depthAttachment = std::make_unique<VulkanRenderPassAttachment>(*prototype);
        return *this;
    }

    VulkanRenderPass& addColorAttachment(const VulkanRenderPassAttachment& colorAttachment) noexcept {
        _colorAttachments.push_back(std::make_unique<VulkanRenderPassAttachment>(colorAttachment));
        return *this;
    }

    VulkanRenderPass& addColorAttachmentAtIndex(const long index, const VulkanRenderPassAttachment& colorAttachment) {
        _colorAttachments.insert(
            _colorAttachments.begin() + index, std::make_unique<VulkanRenderPassAttachment>(colorAttachment)
        );
        return *this;
    }

    VulkanDrawCall& emplaceDrawCall() noexcept {
        return _drawCalls.emplace_back();
    }

    VulkanRenderPass& addTransition(const VulkanPassTransition& transition) {
        if (const auto it = std::ranges::find(_transitions, transition); it == _transitions.end()) {
            _transitions.push_back(transition);
        }
        return *this;
    }

private:
    const VulkanRenderPassDescriptor _passDescriptor;

    VulkanShaderProgram* _shaderProgram = nullptr;

    VulkanPipelineDescriptor      _pipelineDescriptor{};
    const VulkanGraphicsPipeline* _pipeline  = nullptr;

    DescriptorSetsMap     _descriptorSets{};
    DescriptorManagersMap _descriptorManagers{};

    std::unique_ptr<VulkanRenderPassAttachment> _depthAttachment{};

    std::vector<std::unique_ptr<VulkanRenderPassAttachment>> _colorAttachments{};
    std::vector<VulkanPassTransition>                        _transitions{};

    std::vector<VulkanDrawCall> _drawCalls{};
};
