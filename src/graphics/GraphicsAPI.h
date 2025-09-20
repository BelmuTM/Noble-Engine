#pragma once
#ifndef NOBLEENGINE_GRAPHICSAPI_H
#define NOBLEENGINE_GRAPHICSAPI_H

#include "core/platform/Platform.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual bool init(Platform::Window& window) = 0;

    virtual void shutdown() = 0;

    virtual void drawFrame() = 0;
};

#endif //NOBLEENGINE_GRAPHICSAPI_H
