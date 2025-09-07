#pragma once
#ifndef BAZARENGINE_VULKANCONTEXT_H
#define BAZARENGINE_VULKANCONTEXT_H

#include "graphics/GraphicsAPI.h"

#include <vulkan/vulkan.h>

class VulkanContext final : public GraphicsAPI {
public:
    VulkanContext();
    ~VulkanContext() override;

    bool init() override;
    void shutdown() override;
    void drawFrame() override;

private:
    void createInstance();

    VkInstance instance = VK_NULL_HANDLE;
};

#endif //BAZARENGINE_VULKANCONTEXT_H
