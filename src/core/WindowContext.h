#pragma once
#ifndef NOBLEENGINE_WINDOWCONTEXT_H
#define NOBLEENGINE_WINDOWCONTEXT_H

namespace Platform {
    class Window;
}

class InputManager;

struct WindowContext {
    Platform::Window* window       = nullptr;
    InputManager*     inputManager = nullptr;
};

#endif // NOBLEENGINE_WINDOWCONTEXT_H
