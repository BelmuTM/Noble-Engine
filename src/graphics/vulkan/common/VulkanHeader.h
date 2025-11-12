#pragma once
#ifndef NOBLEENGINE_VULKANHEADER_H
#define NOBLEENGINE_VULKANHEADER_H

#define VULKAN_HPP_NO_EXCEPTIONS
#include <vulkan/vulkan.hpp>

#if defined(_WIN32) || defined(_WIN64)

#include <Windows.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan_win32.h>

#undef ERROR

#endif

#define VP_USE_OBJECT
#include <vulkan/vulkan_profiles.hpp>

static constexpr VpProfileProperties vulkanProfile = {
    VP_KHR_ROADMAP_2024_NAME,
    VP_KHR_ROADMAP_2024_SPEC_VERSION
};

constexpr uint32_t VULKAN_VERSION = VK_API_VERSION_1_4;

#endif //NOBLEENGINE_VULKANHEADER_H
