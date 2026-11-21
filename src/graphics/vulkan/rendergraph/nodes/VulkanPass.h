#pragma once

#include "graphics/vulkan/rendergraph/resources/VulkanGraphResource.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorSets.h"

#include <vector>

struct VulkanPassTransition {
    VulkanGraphResource*     resource = nullptr;
    VulkanGraphResourceState src{};
    VulkanGraphResourceState dst{};
};

// Immutable, describes the pass identity
struct VulkanPassDescriptor {
    std::string name = "Undefined_Pass";

    std::string programPath;

    std::vector<VulkanPassResourceUse> resources{};

    [[nodiscard]] std::vector<std::string> getSampledResourcesNames() const {
        std::vector<std::string> names{};

        for (const auto& use : resources) {
            if (use.usage == VulkanGraphResourceUsage::SampledRead) {
                names.push_back(use.name);
            }
        }

        return names;
    }
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

    [[nodiscard]] std::uint32_t getIndex() const noexcept { return _index; }

    [[nodiscard]] const VulkanPassDescriptor& getPassDescriptor() const noexcept { return _passDescriptor; }

    [[nodiscard]]       VulkanDescriptorSets* getDescriptorSets()       noexcept { return _descriptorSets; }
    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _descriptorSets; }

    [[nodiscard]]       TransitionsVector& getEntryTransitions()       noexcept { return _entryTransitions; }
    [[nodiscard]] const TransitionsVector& getEntryTransitions() const noexcept { return _entryTransitions; }

    [[nodiscard]]       TransitionsVector& getExitTransitions()       noexcept { return _exitTransitions; }
    [[nodiscard]] const TransitionsVector& getExitTransitions() const noexcept { return _exitTransitions; }

    // Setters

    void setIndex(const std::uint32_t index) noexcept { _index = index; }

    VulkanPass& setDescriptorSets(VulkanDescriptorSets* descriptorSets) noexcept {
        _descriptorSets = descriptorSets;
        return *this;
    }

    VulkanPass& addEntryTransition(const VulkanPassTransition& entryTransition) {
        _entryTransitions.push_back(entryTransition); return *this;
    }

    VulkanPass& addExitTransition(const VulkanPassTransition& exitTransition) {
        _exitTransitions.push_back(exitTransition); return *this;
    }

private:
    const VulkanPassDescriptor _passDescriptor;

    std::uint32_t _index = 0;

    VulkanDescriptorSets* _descriptorSets = nullptr;

    TransitionsVector _entryTransitions{};
    TransitionsVector _exitTransitions{};
};
