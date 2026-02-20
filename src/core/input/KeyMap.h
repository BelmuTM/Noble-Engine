#pragma once

#include <cstdint>
#include <unordered_map>

enum class InputAction : uint8_t {
    None = 0,

    MoveForward,
    MoveBackward,
    MoveLeft,
    MoveRight,
    MoveUp,
    MoveDown,
    IncreaseSpeed,
    ToggleDebugView
};

class KeyMap {
public:
    KeyMap()  = default;
    ~KeyMap() = default;

    void bind(const int key, const InputAction action) noexcept { _keyBindings[key] = action; }

    InputAction getAction(const int key) const {
        const auto it = _keyBindings.find(key);
        return it != _keyBindings.end() ? it->second : InputAction::None;
    }

private:
    std::unordered_map<int, InputAction> _keyBindings{};
};
