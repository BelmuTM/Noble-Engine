#include "VulkanShaderProgramManager.h"

#include "core/resources/AssetPaths.h"

#include <ranges>

Expected<void> VulkanShaderProgramManager::create(const vk::Device& device) noexcept {
    _device = device;

    return {};
}

void VulkanShaderProgramManager::destroy() noexcept {
    for (const auto& shaderProgram : _programCache | std::views::values) {
        shaderProgram->destroy();
    }

    _device = VK_NULL_HANDLE;
}

Expected<VulkanShaderProgram*> VulkanShaderProgramManager::load(const std::string& path) {
    {
        // Fast path: shader program already in cache
        std::lock_guard lock(_mutex);

        if (const auto cachedProgram = _programCache.find(path); cachedProgram != _programCache.end()) {
            return Expected(cachedProgram->second.get());
        }
    }

    const std::string fullPath = AssetPaths::SHADERS + path;

    VulkanShaderProgram tempProgram{};

    TRY(tempProgram.load(fullPath, _device));

    // Insert shader program into cache
    std::lock_guard lock(_mutex);

    auto [cachedProgram, inserted] =
        _programCache.try_emplace(path, std::make_unique<VulkanShaderProgram>(std::move(tempProgram)));

    return Expected(cachedProgram->second.get());
}
