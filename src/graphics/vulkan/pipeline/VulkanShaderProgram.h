#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/resources/descriptors/VulkanDescriptorManager.h"

#include "graphics/vulkan/pipeline/shader_interface/VulkanPushConstant.h"

#include <unordered_map>
#include <vector>

class VulkanShaderProgram {
public:
    using DescriptorSchemeMap = std::unordered_map<std::uint32_t, VulkanDescriptorScheme>;

    VulkanShaderProgram()  = default;
    ~VulkanShaderProgram() = default;

    VulkanShaderProgram(const VulkanShaderProgram&)            = delete;
    VulkanShaderProgram& operator=(const VulkanShaderProgram&) = delete;

    VulkanShaderProgram(VulkanShaderProgram&&)            noexcept = default;
    VulkanShaderProgram& operator=(VulkanShaderProgram&&) noexcept = default;

    void destroy() noexcept;

    [[nodiscard]] Expected<void> loadFromFiles(const std::vector<std::string>& paths, const vk::Device& device);

    [[nodiscard]] Expected<void> load(const std::string& path, const vk::Device& device);

    [[nodiscard]] const std::vector<vk::PipelineShaderStageCreateInfo>& getStages() const noexcept {
        return _shaderStages;
    }

    [[nodiscard]] const DescriptorSchemeMap& getDescriptorSchemes() const noexcept {
        return _descriptorSchemes;
    }

    [[nodiscard]] const VulkanPushConstantsMap& getPushConstants() const noexcept {
        return _pushConstants;
    }

private:
    void clearShaderModules();

    Expected<vk::ShaderModule> createShaderModule(const std::vector<std::uint32_t>& bytecode) const;

    [[nodiscard]] Expected<void> reflectShaderResources(
        const std::vector<std::uint32_t>& bytecode, vk::ShaderStageFlags stage
    );

    static std::string extractStageExtension(const std::string& path) noexcept;

    static std::vector<std::uint32_t> readShaderSPIRVBytecode(const std::string& path) noexcept;

    static std::vector<std::string> findShaderFilePaths(const std::string& path);

    vk::Device _device{};

    std::vector<vk::ShaderModule>                  _shaderModules{};
    std::vector<vk::PipelineShaderStageCreateInfo> _shaderStages{};

    std::vector<std::string> _stageOutputs{};
    DescriptorSchemeMap      _descriptorSchemes{};
    VulkanPushConstantsMap   _pushConstants{};
};
