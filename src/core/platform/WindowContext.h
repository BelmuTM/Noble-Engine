#pragma once

class Window;
class InputManager;

struct WindowContext {
    Window*       window       = nullptr;
    InputManager* inputManager = nullptr;
};
