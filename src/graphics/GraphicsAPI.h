#pragma once
#ifndef NOBLEENGINE_GRAPHICSAPI_H
#define NOBLEENGINE_GRAPHICSAPI_H

#include "core/Platform.h"
#include "core/Camera.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual bool init(Platform::Window& window) = 0;

    virtual void shutdown() = 0;

    virtual void drawFrame(const Camera& camera) = 0;
};

#endif //NOBLEENGINE_GRAPHICSAPI_H
