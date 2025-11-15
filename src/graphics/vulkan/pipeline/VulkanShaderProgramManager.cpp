#include "VulkanShaderProgramManager.h"

#include "core/debug/ErrorHandling.h"

bool VulkanShaderProgramManager::create(const vk::Device& device, std::string& errorMessage) noexcept {
    _device = device;
    return true;
}

void VulkanShaderProgramManager::destroy() noexcept {
    _device = VK_NULL_HANDLE;
}

bool VulkanShaderProgramManager::load(
    VulkanShaderProgram& program, const std::string& path, const bool fullscreen, std::string& errorMessage
) {
    {
        // If shader program is already cached, return it
        std::lock_guard lock(_mutex);

        if (_cache.contains(path)) {
            program = _cache.at(path);
            return true;
        }
    }

    TRY(program.load(path, fullscreen, _device, errorMessage));

    // Inserting shader program into cache
    std::lock_guard lock(_mutex);
    _cache[path] = program;

    return true;
}
