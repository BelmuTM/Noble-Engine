#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorInfo.h"

class VulkanDescriptorSets;

class VulkanDescriptorManager {
public:
    VulkanDescriptorManager()  = default;
    ~VulkanDescriptorManager() = default;

    VulkanDescriptorManager(const VulkanDescriptorManager&)            = delete;
    VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;

    VulkanDescriptorManager(VulkanDescriptorManager&&)            = delete;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager&&) = delete;

    [[nodiscard]] Expected<void> create(
        const vk::Device&             device,
        const VulkanDescriptorScheme& descriptorScheme,
        std::uint32_t                 framesInFlight,
        std::uint32_t                 setCount
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> allocate(VulkanDescriptorSets*& descriptorSets);

    [[nodiscard]] Expected<void> allocateSets(
        std::vector<vk::DescriptorSet>&      descriptorSets,
        const vk::DescriptorSetAllocateInfo& descriptorSetInfo
    ) const;

    void updateSets(const vk::WriteDescriptorSet& descriptorSetWrite) const;

    [[nodiscard]] std::uint32_t getFramesInFlight() const noexcept { return _framesInFlight; }

    [[nodiscard]] vk::DescriptorSetLayout getLayout() const noexcept { return _descriptorSetLayout; }

    [[nodiscard]] vk::DescriptorPool getPool() const noexcept { return _descriptorPool; }

private:
    void buildDescriptorScheme(const VulkanDescriptorScheme& descriptorScheme);

    [[nodiscard]] Expected<void> createSetLayout();

    [[nodiscard]] Expected<void> createPool();

    vk::Device _device{};

    std::uint32_t _framesInFlight = 0;
    std::uint32_t _setCount        = 0;

    std::vector<vk::DescriptorSetLayoutBinding> _bindings{};
    std::vector<vk::DescriptorPoolSize>         _poolSizes{};

    vk::DescriptorSetLayout _descriptorSetLayout{};
    vk::DescriptorPool      _descriptorPool{};

    std::vector<std::unique_ptr<VulkanDescriptorSets>> _descriptorSets{};
};
