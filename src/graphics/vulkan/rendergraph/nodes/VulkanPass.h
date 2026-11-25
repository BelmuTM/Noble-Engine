#pragma once

#include "VulkanPassDescriptor.h"

#include "graphics/vulkan/pipeline/VulkanPipeline.h"

#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include <vector>

struct VulkanPassTransition {
    VulkanGraphResource*     resource = nullptr;
    VulkanGraphResourceState src{};
    VulkanGraphResourceState dst{};
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

    // Getters

    [[nodiscard]] const VulkanPassDescriptor& getPassDescriptor() const noexcept { return _passDescriptor; }

    [[nodiscard]] std::span<const VulkanPipeline* const> getPipelines() const noexcept { return _pipelines; }

    [[nodiscard]]       VulkanDescriptorSets* getDescriptorSets()       noexcept { return _descriptorSets; }
    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _descriptorSets; }

    [[nodiscard]]       TransitionsVector& getEntryTransitions()       noexcept { return _entryTransitions; }
    [[nodiscard]] const TransitionsVector& getEntryTransitions() const noexcept { return _entryTransitions; }

    [[nodiscard]]       TransitionsVector& getExitTransitions()       noexcept { return _exitTransitions; }
    [[nodiscard]] const TransitionsVector& getExitTransitions() const noexcept { return _exitTransitions; }

    // Setters

    VulkanPass& addPipeline(const VulkanPipeline* pipeline) { _pipelines.push_back(pipeline); return *this; }

    VulkanPass& setDescriptorSets(VulkanDescriptorSets* descriptorSets) noexcept {
        _descriptorSets = descriptorSets; return *this;
    }

    VulkanPass& addEntryTransition(const VulkanPassTransition& entryTransition) {
        _entryTransitions.push_back(entryTransition); return *this;
    }

    VulkanPass& addExitTransition(const VulkanPassTransition& exitTransition) {
        _exitTransitions.push_back(exitTransition); return *this;
    }

private:
    const VulkanPassDescriptor _passDescriptor;

    std::vector<const VulkanPipeline*> _pipelines{};

    VulkanDescriptorSets* _descriptorSets = nullptr;

    TransitionsVector _entryTransitions{};
    TransitionsVector _exitTransitions{};
};
