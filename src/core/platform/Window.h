#pragma once
#ifndef NOBLEENGINE_WINDOW_H
#define NOBLEENGINE_WINDOW_H

#include <string>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    void pollEvents() const;

    void show() const;
    void hide() const;

    [[nodiscard]] GLFWwindow* handle() const noexcept { return _window; }

    [[nodiscard]] int getWidth () const noexcept { return _width ; }
    [[nodiscard]] int getHeight() const noexcept { return _height; }

    void getFramebufferSize(int& width, int& height) const;

    [[nodiscard]] bool isFramebufferResized() const noexcept { return _framebufferResized; }

    void setFramebufferResized(const bool resized) noexcept { _framebufferResized = resized; }

    void setTitle(const std::string& title) const;

private:
    GLFWwindow* _window = nullptr;

    int _width  = 0;
    int _height = 0;

    bool _framebufferResized = false;
};

#endif // NOBLEENGINE_WINDOW_H
