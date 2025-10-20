#pragma once
#ifndef NOBLEENGINE_PLATFORM_H
#define NOBLEENGINE_PLATFORM_H

#include <memory>
#include <string>
#include <vector>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#undef ERROR
#endif

using WindowHandle = void*;

namespace Platform {
    bool init();
    void shutdown();

    bool pollEvents();

    class Window {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        void show() const;
        void hide() const;

        [[nodiscard]] int getWidth () const { return _width ; }
        [[nodiscard]] int getHeight() const { return _height; }

        void getFramebufferSize(int& width, int& height) const;

        [[nodiscard]] bool isFramebufferResized() const { return framebufferResized; }
        void setFramebufferResized(const bool resized) { framebufferResized = resized; }

        void setTitle(const std::string& title) const;

        [[nodiscard]] WindowHandle nativeHandle() const {
            return handle->windowHandle;
        }

#if defined(_WIN32) || defined(_WIN64)
        [[nodiscard]] HINSTANCE hInstance() const {
            return handle ? handle->hInstance : nullptr;
        }
#elif defined(__linux__)
        // TO-DO
#endif

    private:
        int _width;
        int _height;

        bool framebufferResized = false;

        struct NativeHandle {
            WindowHandle windowHandle = nullptr;

#if defined(_WIN32) || defined(_WIN64)
            HINSTANCE hInstance = nullptr;
#elif defined (__linux__)
            // TO-DO
#endif
        };

        std::unique_ptr<NativeHandle> handle;
    };

    std::vector<const char*> getVulkanExtensions();
}

#endif //NOBLEENGINE_PLATFORM_H
