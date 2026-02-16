#pragma once

#include "core/platform/Window.h"

#include "core/entities/camera/Camera.h"

#include "core/resources/AssetManager.h"
#include "core/entities/objects/ObjectManager.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual bool init(
        Window&              window,
        const AssetManager&  assetManager,
        const ObjectManager& objectManager,
        std::string&         errorMessage
    ) = 0;

    virtual void shutdown() = 0;

    virtual void drawFrame(const Camera& camera) = 0;
};
