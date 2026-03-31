#pragma once

#include "core/platform/Window.h"

#include "core/entities/objects/ObjectManager.h"

#include "core/render/FrameUniforms.h"

#include "core/resources/AssetManager.h"

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

    virtual void drawFrame(const FrameUniforms& uniforms) {}
};
