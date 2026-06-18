#pragma once

#include "core/debug/ErrorHandling.h"

#include "graphics/vulkan/common/VulkanHeader.h"

#include "VulkanShaderProgram.h"

#include <memory>
#include <mutex>
#include <unordered_map>

class VulkanShaderProgramManager {
public:
    VulkanShaderProgramManager()  = default;
    ~VulkanShaderProgramManager() = default;

    VulkanShaderProgramManager(const VulkanShaderProgramManager&)            = delete;
    VulkanShaderProgramManager& operator=(const VulkanShaderProgramManager&) = delete;

    VulkanShaderProgramManager(VulkanShaderProgramManager&&)            = delete;
    VulkanShaderProgramManager& operator=(VulkanShaderProgramManager&&) = delete;

    [[nodiscard]] Expected<void> create(const vk::Device& device) noexcept;

    void destroy() noexcept;

    [[nodiscard]] Expected<VulkanShaderProgram*> load(const std::string& path);

private:
    vk::Device _device{};

    std::unordered_map<std::string, std::unique_ptr<VulkanShaderProgram>> _programCache{};

    std::mutex _mutex{};
};
