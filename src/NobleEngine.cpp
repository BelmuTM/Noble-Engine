#include "core/Runtime.h"
#include "core/engine/Engine.h"
#include "core/debug/Logger.h"

#include "core/platform/Platform.h"
#include "core/platform/SignalHandlers.h"

#include <atomic>

std::atomic running(true);

int main() {
    SignalHandlers::setupHandlers(running);

    Logger::Manager loggerManager;

    std::string errorMessage;

    if (!Platform::init(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    Scene sceneSponza;
    sceneSponza.addObject("lucy.obj", {-1.0f, 1.0f, 1.47f}, {0.0f, 0.0f, -30.0f}, glm::vec3{0.0025f});
    sceneSponza.addObject("stanford_dragon.obj", {3.0f, 0.7f, 0.6f}, {0.0f, 180.0f, 60.0f}, glm::vec3{0.6f});
    sceneSponza.addObject("stanford_bunny.obj", {-3.0f, 1.0f, -0.25f}, {0.0f, 90.0f, 0.0f}, glm::vec3{7.0f});
    sceneSponza.addObject("happy.obj", {-4.5f, -0.4f, -0.36f}, {0.0f, 120.0f, 0.0f}, glm::vec3{7.0f});
    sceneSponza.addObject("sponza_old.gltf", {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, glm::vec3{1.0f});

    /*
    Scene sceneBnuuy;
    static constexpr uint32_t bnuuyPerAxis = 16;
    static constexpr float    bnuuyOffset  = 2.0f;

    for (int32_t x = 0; x < bnuuyPerAxis; x++) {
        for (int32_t y = 0; y < bnuuyPerAxis; y++) {
            float xPosition = (x - (bnuuyPerAxis - 1) / 2.0f) * bnuuyOffset;
            float yPosition = (y - (bnuuyPerAxis - 1) / 2.0f) * bnuuyOffset;

            sceneBnuuy.addObject(
                "stanford_bunny.obj", {xPosition, yPosition, 0.0f}, {90.0f, 90.0f, 0.0f}, glm::vec3{7.0f}
            );
        }
    }
    */

    Runtime runtime(sceneSponza, running);

    if (!runtime.init(errorMessage)) {
        Engine::fatalExit(errorMessage);
    }

    runtime.run();

    runtime.shutdown();

    Platform::shutdown();

    return 0;
}
