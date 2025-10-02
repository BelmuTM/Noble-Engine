#pragma once
#ifndef NOBLEENGINE_VULKANHEADER_H
#define NOBLEENGINE_VULKANHEADER_H

#if defined(_WIN32) || defined(_WIN64)
#define VK_USE_PLATFORM_WIN32_KHR
#endif

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#undef ERROR

#endif //NOBLEENGINE_VULKANHEADER_H