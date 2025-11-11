#pragma once
#ifndef NOBLEENGINE_GRAPHICSAPI_H
#define NOBLEENGINE_GRAPHICSAPI_H

#include "core/objects/object/Object.h"
#include "core/Platform.h"
#include "core/objects/camera/Camera.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual bool init(Platform::Window& window, const std::vector<Object>& objects) = 0;

    virtual void shutdown() = 0;

    virtual void drawFrame(const Camera& camera) = 0;
};

#endif //NOBLEENGINE_GRAPHICSAPI_H
