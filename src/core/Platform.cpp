#include "Platform.h"

#include "ResourceManager.h"
#include "WindowContext.h"

#include "common/StbUsage.h"

namespace Platform {
    bool init() {
        if (!glfwInit()) return false;
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        return true;
    }

    void shutdown() {
        glfwTerminate();
    }

    void Window::pollEvents() const {
        while (!glfwWindowShouldClose(_window)) {
            glfwPollEvents();
        }
    }

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

    void Window::show() const {
        if (_window) glfwShowWindow(_window);
    }

    void Window::hide() const {
        if (_window) glfwHideWindow(_window);
    }

    void Window::getFramebufferSize(int& width, int& height) const {
        glfwGetFramebufferSize(_window, &width, &height);
    }

    void Window::setTitle(const std::string& title) const {
        glfwSetWindowTitle(_window, title.c_str());
    }

    std::vector<const char*> getVulkanExtensions() {
        uint32_t count;
        const char** extensions = glfwGetRequiredInstanceExtensions(&count);
        if (!extensions || count == 0) return {};
        return {extensions, extensions + count};
    }
}
