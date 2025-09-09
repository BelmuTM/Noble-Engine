#pragma once
#ifndef BAZARENGINE_VULKANCONTEXT_H
#define BAZARENGINE_VULKANCONTEXT_H

#include "graphics/GraphicsAPI.h"

#include <vulkan/vulkan.h>

class VulkanContext final : public GraphicsAPI {
public:
    ~VulkanContext() override;

    bool init() override;
    void shutdown() override;
    void drawFrame() override;

private:
    VkInstance               instance       = VK_NULL_HANDLE;
    VkDebugUtilsMessengerEXT debugMessenger = VK_NULL_HANDLE;
    VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
    VkDevice                 logicalDevice  = VK_NULL_HANDLE;
    VkQueue                  graphicsQueue  = VK_NULL_HANDLE;

    static std::vector<const char*> getRequiredExtensions();

    void createInstance();
    void setupDebugMessenger();

    void pickPhysicalDevice();
    void createLogicalDevice();

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      severity,
                                                        VkDebugUtilsMessageTypeFlagsEXT             type,
                                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                                        void*);

    static VkResult createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                 const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                 const VkAllocationCallbacks*              pAllocator,
                                                 VkDebugUtilsMessengerEXT*                 pMessenger);

    static void destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                              VkDebugUtilsMessengerEXT     messenger,
                                              const VkAllocationCallbacks* pAllocator);
};

#endif //BAZARENGINE_VULKANCONTEXT_H
