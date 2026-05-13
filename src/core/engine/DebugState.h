#pragma once

struct DebugState {
    static constexpr std::uint8_t MAX_DEBUG_MODES = 5;

    int debugMode = 0;

    void incrementMode() {
        debugMode = (debugMode + 1) % MAX_DEBUG_MODES;
    }
};
