#include "VulkanDebugger.h"

#include <string>

namespace VulkanDebugger {
    std::string formatVulkanErrorMessage(const std::string& expression, const vk::Result result) {
        const auto errorCode = static_cast<int32_t>(result);
        return expression + " failed: " + vk::to_string(result) + " (error code: " + std::to_string(errorCode) + ").";
    }

#if defined(VULKAN_DEBUG_UTILS)

    void beginLabel(const vk::CommandBuffer commandBuffer, const std::string& name, const glm::vec3& color) {
        vk::DebugUtilsLabelEXT label{};
        label
            .setPLabelName(name.c_str())
            .setColor({color.r, color.g, color.b, 1.0f});
        commandBuffer.beginDebugUtilsLabelEXT(label);
    }

    void endLabel(const vk::CommandBuffer commandBuffer) {
        commandBuffer.endDebugUtilsLabelEXT();
    }

#else

    void beginLabel(const vk::CommandBuffer commandBuffer, const std::string& name, const glm::vec3& color) {}
    void endLabel(const vk::CommandBuffer commandBuffer) {}

#endif
}
