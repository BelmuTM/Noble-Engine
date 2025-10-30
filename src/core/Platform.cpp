#include "Platform.h"

#include "core/Engine.h"
#include "core/ResourceManager.h"

#include "graphics/vulkan/resources/StbUsage.h"

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

    Window::Window(const int width, const int height, const std::string& title) {
        _width  = width;
        _height = height;

        _window = glfwCreateWindow(_width, _height, title.c_str(), nullptr, nullptr);

        GLFWimage images[1];
        images[0].pixels = stbi_load(iconPath.c_str(), &images[0].width, &images[0].height, nullptr, STBI_rgb_alpha);

        glfwSetWindowIcon(_window, 1, images);

        stbi_image_free(images[0].pixels);
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
        return {extensions, extensions + count};
    }
}
