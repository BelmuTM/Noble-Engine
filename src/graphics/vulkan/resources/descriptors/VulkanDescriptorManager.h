#pragma once

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

    [[nodiscard]] bool create(
        const vk::Device&             device,
        const VulkanDescriptorScheme& descriptorScheme,
        uint32_t                      framesInFlight,
        uint32_t                      maxSets,
        std::string&                  errorMessage
    ) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool allocate(VulkanDescriptorSets*& descriptorSets, std::string& errorMessage);

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
    void buildDescriptorScheme(const VulkanDescriptorScheme& descriptorScheme);

    [[nodiscard]] bool createSetLayout(std::string& errorMessage);

    [[nodiscard]] bool createPool(std::string& errorMessage);

    vk::Device _device{};

    uint32_t _framesInFlight = 0;
    uint32_t _maxSets        = 0;

    std::vector<vk::DescriptorSetLayoutBinding> _bindings{};
    std::vector<vk::DescriptorPoolSize>         _poolSizes{};

    vk::DescriptorSetLayout _descriptorSetLayout{};
    vk::DescriptorPool      _descriptorPool{};

    std::vector<std::unique_ptr<VulkanDescriptorSets>> _descriptorSets{};
};
