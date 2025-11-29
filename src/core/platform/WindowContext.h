#pragma once
#ifndef NOBLEENGINE_WINDOWCONTEXT_H
#define NOBLEENGINE_WINDOWCONTEXT_H

class Window;
class InputManager;

struct WindowContext {
    Window*       window       = nullptr;
    InputManager* inputManager = nullptr;
};

#endif // NOBLEENGINE_WINDOWCONTEXT_H
