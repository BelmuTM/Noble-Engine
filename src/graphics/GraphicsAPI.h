#pragma once
#ifndef BAZARENGINE_GRAPHICSAPI_H
#define BAZARENGINE_GRAPHICSAPI_H

#include "core/platform/Platform.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual bool init(const Platform::Window& window) = 0;
    virtual void               shutdown() = 0;
    virtual void               drawFrame() = 0;
};

#endif //BAZARENGINE_GRAPHICSAPI_H
