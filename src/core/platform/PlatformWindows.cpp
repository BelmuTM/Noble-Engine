#define UNICODE

#include "Platform.h"
#include "core/Engine.h"
#include "../res/resource.h"

#include <iostream>

#include <Windows.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>

static HINSTANCE g_hInstance = nullptr;

static constexpr wchar_t CLASS_NAME[] = L"EngineWindowClass";

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
        g_hInstance = GetModuleHandle(nullptr);
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

    Window::Window(const int width, const int height, const std::string &title) {
        static bool registered = false;
        if (!registered) {
            WNDCLASS wc      = {};
            wc.lpfnWndProc   = windowProc;
            wc.hInstance     = g_hInstance;
            wc.hIcon         = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_ICON));
            wc.lpszClassName = CLASS_NAME;

            if (!RegisterClass(&wc)) {
                Engine::fatalExit("Failed to register Window class");
            }

            registered = true;
        }

        const auto wTitle = std::wstring(title.begin(), title.end());

        handle = CreateWindowEx(
        0,
        CLASS_NAME,
        wTitle.c_str(),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, width, height,
        nullptr, nullptr, g_hInstance, nullptr
        );

        if (!handle) {
            Engine::fatalExit("Failed to create window");
        }
    }

    Window::~Window() {
        if (handle) {
            DestroyWindow(static_cast<HWND>(handle));
            handle = nullptr;
        }
    }

    void Window::show() const {
        ShowWindow(static_cast<HWND>(handle), SW_SHOW);
    }

    void Window::hide() const {
        ShowWindow(static_cast<HWND>(handle), SW_HIDE);
    }

    std::vector<const char*> getVulkanExtensions() {
        return {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_WIN32_SURFACE_EXTENSION_NAME
        };
    }
}
