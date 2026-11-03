#include "core/Runtime.h"
#include "core/debug/Logger.h"
#include "core/engine/Engine.h"
#include "core/multithreading/ThreadRegistry.h"

#include "core/platform/Platform.h"
#include "core/platform/SignalHandlers.h"

int main() {
    SignalHandlers::setupHandlers(Engine::running);

    ThreadScope mainScope("MainThread");

    Logger::Manager loggerManager;

    Engine::fatalOnFail(Platform::init());

    Scene sceneSponza;
    sceneSponza.addObject("lucy.obj", {-1.0f, 1.0f, 1.47f}, {0.0f, 0.0f, -30.0f}, glm::vec3{0.0025f});
    sceneSponza.addObject("stanford_dragon.obj", {3.0f, 0.7f, 0.6f}, {0.0f, 180.0f, 60.0f}, glm::vec3{0.6f});
    sceneSponza.addObject("stanford_bunny.obj", {-3.0f, 1.0f, -0.25f}, {90.0f, 90.0f, 0.0f}, glm::vec3{7.0f});
    sceneSponza.addObject("happy.obj", {-4.5f, -0.4f, -0.36f}, {90.0f, 120.0f, 0.0f}, glm::vec3{7.0f});
    sceneSponza.addObject("teapot.obj", {1.0f, 4.0f, 0.0f}, {90.0f, 0.0f, 0.0f}, glm::vec3{0.015f});
    sceneSponza.addObject("sponza_old.gltf", {0.0f, 0.0f, 0.0f}, {90.0f, 0.0f, 0.0f}, glm::vec3{1.0f});

    Scene sceneBistro;
    sceneBistro.addObject("bistro.gltf", {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, glm::vec3{1.0f});

    Scene sceneDragons;

    std::default_random_engine gen;
    std::uniform_real_distribution<float> distribution(0.0, 1.0);

    static constexpr float maxRange = 128.0f;
    static constexpr float maxAngle = 360.0f;

    for (std::uint32_t x = 0; x < 2048; x++) {
        float xPosition = distribution(gen) * maxRange - maxRange * 0.5;
        float yPosition = distribution(gen) * maxRange - maxRange * 0.5;
        float zPosition = distribution(gen) * maxRange - maxRange * 0.5;

        float xRotation = distribution(gen) * maxAngle;
        float yRotation = distribution(gen) * maxAngle;
        float zRotation = distribution(gen) * maxAngle;

        sceneDragons.addObject(
            "stanford_dragon.obj", {xPosition, yPosition, zPosition}, {xRotation, yRotation, zRotation}, glm::vec3{1.0f}
        );
    }

    Runtime runtime(sceneSponza, Engine::running);

    Engine::fatalOnFail(runtime.init());

    runtime.run();

    runtime.shutdown();

    Platform::shutdown();

    return Engine::exitCode;
}
