#pragma once

#include "graphics/vulkan/rendergraph/nodes/VulkanRenderPass.h"

#include "graphics/vulkan/resources/frame/VulkanFrameResources.h"
#include "graphics/vulkan/resources/ssbo/VulkanStorageBufferManager.h"

class VulkanFrameCuller {
public:
    static constexpr std::uint32_t MAX_DRAWS = 5'000'000;

    VulkanFrameCuller()  = default;
    ~VulkanFrameCuller() = default;

    VulkanFrameCuller(const VulkanFrameCuller&)            = delete;
    VulkanFrameCuller& operator=(const VulkanFrameCuller&) = delete;

    VulkanFrameCuller(VulkanFrameCuller&&)            = delete;
    VulkanFrameCuller& operator=(VulkanFrameCuller&&) = delete;

    [[nodiscard]] Expected<void> create(
        const VulkanDevice&         device,
        VulkanStorageBufferManager& storageBufferManager,
        std::uint32_t               framesInFlight
    ) noexcept;

    void destroy() noexcept;

    Expected<void> cull(const std::vector<std::unique_ptr<VulkanRenderPass>>& passes, const FrameUniforms& uniforms);

    [[nodiscard]] const std::vector<VulkanDrawCall*>& getDrawCalls(const VulkanRenderPass* pass) const {
        return _visibleDrawCalls.at(pass);
    }

    [[nodiscard]] const std::uint32_t& getIndirectionOffset(const VulkanRenderPass* pass) const {
        return _indirectionOffsets.at(pass);
    }

    [[nodiscard]] static VulkanDescriptorScheme getDescriptorScheme() noexcept {
        static const VulkanDescriptorScheme scheme = {
            {0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex}
        };
        return scheme;
    }

    [[nodiscard]] const VulkanDescriptorManager& getDescriptorManager() const noexcept { return _descriptorManager; }

    [[nodiscard]] VulkanStorageBuffer* getIndirectionBuffer() const noexcept { return _indirectionBuffer; }

    [[nodiscard]] const VulkanDescriptorSets* getDescriptorSets() const noexcept { return _indirectionDescriptors; }

private:
    std::unordered_map<const VulkanRenderPass*, std::vector<VulkanDrawCall*>> _visibleDrawCalls{};

    std::unordered_map<const VulkanRenderPass*, std::uint32_t> _indirectionOffsets{};

    VulkanDescriptorManager _descriptorManager{};

    VulkanStorageBuffer*  _indirectionBuffer      = nullptr;
    VulkanDescriptorSets* _indirectionDescriptors = nullptr;
};
