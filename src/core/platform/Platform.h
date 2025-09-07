#pragma once
#ifndef BAZARENGINE_PLATFORM_H
#define BAZARENGINE_PLATFORM_H

#include <string>
#include <vector>

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

        [[nodiscard]] WindowHandle nativeHandle() const {
            return handle;
        }

    private:
        WindowHandle handle;
    };

    std::vector<const char*> getVulkanExtensions();
}

#endif //BAZARENGINE_PLATFORM_H
