#pragma once

#include "core/engine/DebugState.h"
#include "core/entities/camera/Camera.h"

#include <chrono>

#include <glm/glm.hpp>

struct alignas(16) FrameUniforms {
    glm::mat4 viewMatrix;
    glm::mat4 viewInverseMatrix;

    glm::mat4 projectionMatrix;
    glm::mat4 projectionInverseMatrix;

    glm::vec3 cameraPosition;

    float nearPlane;
    float farPlane;

    float frameTimeCounter;
    float frameCounter;

    float viewWidth;
    float viewHeight;

    int debugMode = 0;

    void update(Camera& camera, const std::uint32_t width, const std::uint32_t height, const DebugState& debugState) {
        static auto startTime   = std::chrono::high_resolution_clock::now();
        const  auto currentTime = std::chrono::high_resolution_clock::now();

        viewMatrix        = camera.getViewMatrix();
        viewInverseMatrix = camera.getViewInverseMatrix();

        projectionMatrix        = camera.getProjectionMatrix();
        projectionInverseMatrix = camera.getProjectionInverseMatrix();

        // Vulkan NDC projection matrix flip
        projectionMatrix[1][1] *= -1;

        cameraPosition = camera.getPosition();

        nearPlane = camera.getNearPlane();
        farPlane  = camera.getFarPlane();

        frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

        ++frameCounter;

        viewWidth  = static_cast<float>(width);
        viewHeight = static_cast<float>(height);

        debugMode = debugState.debugMode;
    }
};
