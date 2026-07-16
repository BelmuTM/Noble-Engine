#pragma once

#include "graphics/vulkan/pipeline/VulkanPipeline.h"
#include "graphics/vulkan/pipeline/shaders/VulkanShaderProgram.h"

#include "graphics/vulkan/rendergraph/resources/VulkanGraphicsPassAttachment.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"
#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include <memory>
#include <ranges>

struct VulkanPassTransition {
    VulkanPassResource* resource     = nullptr;
    vk::ImageLayout     targetLayout = vk::ImageLayout::eUndefined;

    bool operator==(const VulkanPassTransition& other) const {
        return resource == other.resource && targetLayout == other.targetLayout;
    }
};

// Immutable, describes the pass identity
struct VulkanPassDescriptor {
    std::string name = "Undefined_Pass";

    std::string programPath;

    std::vector<VulkanGraphicsPassAttachmentDescriptor> readDescriptors{};
    std::vector<VulkanGraphicsPassAttachmentDescriptor> writeDescriptors{};
};

// Mutable, describes the pass state (data subject to hot reloading)
class VulkanPass {
public:
    using TransitionsVector = std::vector<VulkanPassTransition>;

    explicit VulkanPass(VulkanPassDescriptor descriptor) : _passDescriptor(std::move(descriptor)) {}

    ~VulkanPass() = default;

    VulkanPass(const VulkanPass&)            = delete;
    VulkanPass& operator=(const VulkanPass&) = delete;

    VulkanPass(VulkanPass&&)            = delete;
    VulkanPass& operator=(VulkanPass&&) = delete;

    void destroy() const noexcept;

    // Getters

    [[nodiscard]] const VulkanPassDescriptor& getPassDescriptor() const noexcept { return _passDescriptor; }

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

    [[nodiscard]]       TransitionsVector& getEntryTransitions()       noexcept { return _entryTransitions; }
    [[nodiscard]] const TransitionsVector& getEntryTransitions() const noexcept { return _entryTransitions; }

    [[nodiscard]]       TransitionsVector& getExitTransitions()       noexcept { return _exitTransitions; }
    [[nodiscard]] const TransitionsVector& getExitTransitions() const noexcept { return _exitTransitions; }

    // Setters

    VulkanPass& setShaderProgram(VulkanShaderProgram* shaderProgram) noexcept {
        _shaderProgram = shaderProgram;
        return *this;
    }

    VulkanPass& setDescriptorManager(std::unique_ptr<VulkanDescriptorManager> descriptorManager) noexcept {
        _descriptorManager = std::move(descriptorManager);
        return *this;
    }

    VulkanPass& setDescriptorSets(VulkanDescriptorSets* descriptorSets) noexcept {
        _descriptorSets = descriptorSets;
        return *this;
    }

    VulkanPass& setPipelineLayoutDescriptor(
        const VulkanPipelineLayoutDescriptor& pipelineLayoutDescriptor
    ) noexcept {
        _pipelineLayoutDescriptor = pipelineLayoutDescriptor;
        return *this;
    }

    VulkanPass& addEntryTransition(const VulkanPassTransition& entryTransition) {
        const auto cachedTransition = std::ranges::find(_entryTransitions, entryTransition);
        if (cachedTransition == _entryTransitions.end()) {
            _entryTransitions.push_back(entryTransition);
        }
        return *this;
    }

    VulkanPass& addExitTransition(const VulkanPassTransition& exitTransition) {
        const auto cachedTransition = std::ranges::find(_exitTransitions, exitTransition);
        if (cachedTransition == _exitTransitions.end()) {
            _exitTransitions.push_back(exitTransition);
        }
        return *this;
    }

private:
    const VulkanPassDescriptor _passDescriptor;

    VulkanShaderProgram* _shaderProgram = nullptr;

    VulkanPipelineLayoutDescriptor _pipelineLayoutDescriptor{};

    std::unique_ptr<VulkanDescriptorManager> _descriptorManager{};
    VulkanDescriptorSets*                    _descriptorSets = nullptr;

    TransitionsVector _entryTransitions{};
    TransitionsVector _exitTransitions{};
};
