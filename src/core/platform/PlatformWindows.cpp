#define UNICODE

// Windows 10 flags
#define WINVER       0x0A00
#define _WIN32_WINNT 0x0A00

#include "Platform.h"
#include "core/Engine.h"
#include "../res/resource.h"

#include <cmath>

#include <Windows.h>
#undef ERROR

#include <vulkan/vulkan_win32.h>

#include "core/debug/Logger.h"

static HINSTANCE g_hInstance = nullptr;

static constexpr wchar_t CLASS_NAME[] = L"NobleEngine_Window";

std::vector<const char*> Platform::getVulkanExtensions() {
    return {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
}

LRESULT CALLBACK windowProc(const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

namespace Platform {
    bool init() {
        if (!g_hInstance) g_hInstance = GetModuleHandle(nullptr);
        return true;
    }

    void shutdown() {
    }

    bool pollEvents() {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                return false;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }

    Window::Window(const int width, const int height, const std::string& title) {
        _width  = width;
        _height = height;

        if (!g_hInstance) {
            g_hInstance = GetModuleHandle(nullptr);
        }

        handle = std::make_unique<NativeHandle>();
        handle->hInstance = g_hInstance;

        static bool registered = false;
        if (!registered) {
            WNDCLASS wc{};
            wc.lpfnWndProc   = windowProc;
            wc.hInstance     = g_hInstance;
            wc.hIcon         = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON));
            wc.lpszClassName = CLASS_NAME;

            if (!RegisterClass(&wc)) {
                Engine::fatalExit("Failed to register window class");
            }

            registered = true;
        }

        const auto wTitle = std::wstring(title.begin(), title.end());

        handle->windowHandle = CreateWindowEx(
            0,
            CLASS_NAME,
            wTitle.c_str(),
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, width, height,
            nullptr, nullptr, g_hInstance, nullptr
        );

        if (!handle->windowHandle) {
            Engine::fatalExit("Failed to create window");
        }
    }

    Window::~Window() {
        if (handle) {
            if (handle->windowHandle) DestroyWindow(static_cast<HWND>(handle->windowHandle));
            handle->windowHandle = nullptr;
        }
    }

    void Window::show() const {
        ShowWindow(static_cast<HWND>(handle->windowHandle), SW_SHOW);
    }

    void Window::hide() const {
        ShowWindow(static_cast<HWND>(handle->windowHandle), SW_HIDE);
    }

    void Window::getFrameBufferSize(int& width, int& height) const {
        if (!handle || !handle->windowHandle) {
            Logger::error("Failed to get framebuffer size: window handle is null");
            return;
        }
        const auto hwnd = static_cast<HWND>(handle->windowHandle);

        RECT rect;
        if (!GetClientRect(hwnd, &rect)) {
            width  = 0;
            height = 0;
            return;
        }
        const int logicalWidth  = rect.right  - rect.left;
        const int logicalHeight = rect.bottom - rect.top;

        const int   dpi   = static_cast<int>(GetDpiForWindow(hwnd));
        const float scale = static_cast<float>(dpi) / 96.0f;

        width  = static_cast<int>(std::lround(logicalWidth  * scale));
        height = static_cast<int>(std::lround(logicalHeight * scale));
    }
}
