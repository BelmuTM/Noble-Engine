#pragma once

#include "graphics/vulkan/common/VulkanHeader.h"

#include "graphics/vulkan/pipeline/VulkanShaderProgram.h"

#include <memory>
#include <unordered_map>

class VulkanShaderProgramManager {
public:
    VulkanShaderProgramManager()  = default;
    ~VulkanShaderProgramManager() = default;

    VulkanShaderProgramManager(const VulkanShaderProgramManager&)            = delete;
    VulkanShaderProgramManager& operator=(const VulkanShaderProgramManager&) = delete;

    VulkanShaderProgramManager(VulkanShaderProgramManager&&)            = delete;
    VulkanShaderProgramManager& operator=(VulkanShaderProgramManager&&) = delete;

    [[nodiscard]] bool create(const vk::Device& device, std::string& errorMessage) noexcept;

    void destroy() noexcept;

    [[nodiscard]] bool load(VulkanShaderProgram*& program, const std::string& path, std::string& errorMessage);

private:
    vk::Device _device{};

    std::unordered_map<std::string, std::unique_ptr<VulkanShaderProgram>> _cache{};

    std::mutex _mutex{};
};
