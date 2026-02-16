#include "VulkanShaderProgramManager.h"

#include "core/debug/ErrorHandling.h"

#include <ranges>

bool VulkanShaderProgramManager::create(const vk::Device& device, std::string& errorMessage) noexcept {
    _device = device;

    return true;
}

void VulkanShaderProgramManager::destroy() noexcept {
    for (const auto& shaderProgram : _cache | std::views::values) {
        shaderProgram->destroy();
    }

    _device = VK_NULL_HANDLE;
}

bool VulkanShaderProgramManager::load(
    VulkanShaderProgram*& program, const std::string& path, std::string& errorMessage
) {
    {
        // If shader program is already cached, return it
        std::lock_guard lock(_mutex);

        if (_cache.contains(path)) {
            program = _cache.at(path).get();
            return true;
        }
    }

    VulkanShaderProgram tempProgram{};

    TRY(tempProgram.load(path, _device, errorMessage));

    // Insert shader program into cache
    std::lock_guard lock(_mutex);

    auto [it, inserted] = _cache.try_emplace(path, std::make_unique<VulkanShaderProgram>(std::move(tempProgram)));

    program = it->second.get();

    return true;
}
