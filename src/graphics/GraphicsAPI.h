#pragma once

#include "core/debug/ErrorHandling.h"
#include "core/platform/Window.h"

#include "core/entities/objects/ObjectManager.h"

#include "core/render/FrameUniforms.h"

#include "core/resources/AssetManager.h"

class GraphicsAPI {
public:
    GraphicsAPI()          = default;
    virtual ~GraphicsAPI() = default;

    [[nodiscard]] virtual Expected<void> init(
        Window&              window,
        const AssetManager&  assetManager,
        const ObjectManager& objectManager
    ) = 0;

    virtual void shutdown() = 0;

    virtual Expected<void> drawFrame(const FrameUniforms& uniforms) = 0;
};
