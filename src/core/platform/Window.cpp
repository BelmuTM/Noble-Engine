#include "Window.h"

#include "WindowContext.h"

#include "core/common/libraries/stbUsage.h"
#include "core/engine/ResourceManager.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    auto context = static_cast<WindowContext*>(glfwGetWindowUserPointer(window));
    if (context->window) context->window->setFramebufferResized(true);
}

Window::Window(const int width, const int height, const std::string& title) {
    _width  = width;
    _height = height;

    _window = glfwCreateWindow(_width, _height, title.c_str(), nullptr, nullptr);

    GLFWimage images[1];
    images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, nullptr, STBI_rgb_alpha);

    glfwSetWindowIcon(_window, 1, images);
    stbi_image_free(images[0].pixels);

    glfwSetFramebufferSizeCallback(_window, framebufferSizeCallback);
}

Window::~Window() {
    if (_window) glfwDestroyWindow(_window);
}

void Window::close() const {
    if (_window) glfwSetWindowShouldClose(_window, GLFW_TRUE);
}

void Window::pollEvents() const {
    if (!_window) return;

    while (!glfwWindowShouldClose(_window)) {
        glfwPollEvents();
    }
}

void Window::show() const {
    if (_window) glfwShowWindow(_window);
}

void Window::hide() const {
    if (_window) glfwHideWindow(_window);
}

void Window::getFramebufferSize(int& width, int& height) const {
    if (_window) glfwGetFramebufferSize(_window, &width, &height);
}

void Window::setTitle(const std::string& title) const {
    if (_window) glfwSetWindowTitle(_window, title.c_str());
}
