#include "VulkanShaderProgramManager.h"

#include "core/resources/AssetPaths.h"

#include <ranges>

Expected<void> VulkanShaderProgramManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanShaderProgramManager::destroy() noexcept {
    for (const auto& shaderProgram : _cache | std::views::values) {
        shaderProgram->destroy();
    }

    _device = VK_NULL_HANDLE;
}

Expected<void> VulkanShaderProgramManager::load(VulkanShaderProgram*& program, const std::string& path) {
    {
        // Fast path: shader program already in cache
        std::lock_guard lock(_mutex);

        if (_cache.contains(path)) {
            program = _cache.at(path).get();
            return {};
        }
    }

    const std::string fullPath = AssetPaths::SHADERS + path;

    VulkanShaderProgram tempProgram{};

    TRY(tempProgram.load(fullPath, _device));

    // Insert shader program into cache
    std::lock_guard lock(_mutex);

    auto [cachedProgram, inserted] =
        _cache.try_emplace(path, std::make_unique<VulkanShaderProgram>(std::move(tempProgram)));

    program = cachedProgram->second.get();

    return {};
}
