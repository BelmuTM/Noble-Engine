#pragma once
#ifndef NOBLEENGINE_VULKANHEADER_H
#define NOBLEENGINE_VULKANHEADER_H

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>
#endif

#undef ERROR

constexpr uint32_t VULKAN_VERSION = VK_API_VERSION_1_4;

#endif //NOBLEENGINE_VULKANHEADER_H
