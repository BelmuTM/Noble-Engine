#pragma once
#ifndef NOBLEENGINE_GRAPHICSAPI_H
#define NOBLEENGINE_GRAPHICSAPI_H

#include "core/entities/camera/Camera.h"
#include "core/entities/objects/ObjectManager.h"
#include "core/platform/Window.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual bool init(Window& window, const ObjectManager& objectManager, std::string& errorMessage) = 0;

    virtual void shutdown() = 0;

    virtual void drawFrame(const Camera& camera) = 0;
};

#endif //NOBLEENGINE_GRAPHICSAPI_H
