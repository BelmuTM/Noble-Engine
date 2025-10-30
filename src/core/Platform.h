#pragma once
#ifndef NOBLEENGINE_PLATFORM_H
#define NOBLEENGINE_PLATFORM_H

#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace Platform {
    bool init();
    void shutdown();

    class Window {
    public:
        Window(int width, int height, const std::string& title);
        ~Window();

        void pollEvents() const;

        void show() const;
        void hide() const;

        [[nodiscard]] GLFWwindow* handle() const { return _window; }

        [[nodiscard]] int getWidth () const { return _width ; }
        [[nodiscard]] int getHeight() const { return _height; }

        void getFramebufferSize(int& width, int& height) const;

        [[nodiscard]] bool isFramebufferResized() const { return _framebufferResized; }
        void setFramebufferResized(const bool resized) { _framebufferResized = resized; }

        void setTitle(const std::string& title) const;

    private:
        GLFWwindow* _window;

        int _width;
        int _height;

        bool _framebufferResized = false;
    };

    std::vector<const char*> getVulkanExtensions();
}

#endif //NOBLEENGINE_PLATFORM_H
