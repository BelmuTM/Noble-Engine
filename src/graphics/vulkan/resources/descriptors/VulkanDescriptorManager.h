#pragma once
#ifndef NOBLEENGINE_VULKANDESCRIPTORMANAGER_H
#define NOBLEENGINE_VULKANDESCRIPTORMANAGER_H

#include "graphics/vulkan/common/VulkanHeader.h"

struct DescriptorBindingInfo {
    // uint32_t             binding;
    vk::DescriptorType   type;
    vk::ShaderStageFlags stageFlags;
    uint32_t             count = 1;
};

class VulkanDescriptorManager {
public:
    using DescriptorScheme = std::vector<DescriptorBindingInfo>;

    VulkanDescriptorManager()  = default;
    ~VulkanDescriptorManager() = default;

    VulkanDescriptorManager(const VulkanDescriptorManager&)            = delete;
    VulkanDescriptorManager& operator=(const VulkanDescriptorManager&) = delete;

    VulkanDescriptorManager(VulkanDescriptorManager&&)            = delete;
    VulkanDescriptorManager& operator=(VulkanDescriptorManager&&) = delete;

    [[nodiscard]] bool create(
        const vk::Device&       device,
        const DescriptorScheme& descriptorScheme,
        uint32_t                framesInFlight,
        uint32_t                maxSets,
        std::string&            errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool allocateSets(
        std::vector<vk::DescriptorSet>&      descriptorSets,
        const vk::DescriptorSetAllocateInfo& descriptorSetInfo,
        std::string&                         errorMessage
    ) const;

    void updateSets(const vk::WriteDescriptorSet& descriptorSetWrite) const;

    [[nodiscard]] uint32_t getFramesInFlight() const noexcept { return _framesInFlight; }

    [[nodiscard]] vk::DescriptorSetLayout getLayout() const noexcept { return _descriptorSetLayout; }

    [[nodiscard]] vk::DescriptorPool getPool() const noexcept { return _descriptorPool; }

private:
    vk::Device _device{};

    uint32_t _framesInFlight = 0;
    uint32_t _maxSets        = 0;

    std::vector<vk::DescriptorSetLayoutBinding> _bindings{};
    std::vector<vk::DescriptorPoolSize>         _poolSizes{};

    vk::DescriptorSetLayout _descriptorSetLayout{};
    vk::DescriptorPool      _descriptorPool{};

    void buildDescriptorScheme(const DescriptorScheme& descriptorScheme);

    [[nodiscard]] bool createSetLayout(std::string& errorMessage);
    [[nodiscard]] bool createPool(std::string& errorMessage);
};

#endif //NOBLEENGINE_VULKANDESCRIPTORMANAGER_H
