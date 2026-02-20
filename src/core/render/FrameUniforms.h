#pragma once

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

    void update(const Camera& camera, const uint32_t width, const uint32_t height) {
        static auto startTime   = std::chrono::high_resolution_clock::now();
        const  auto currentTime = std::chrono::high_resolution_clock::now();

        viewWidth  = static_cast<float>(width);
        viewHeight = static_cast<float>(height);

        viewMatrix        = camera.getViewMatrix();
        viewInverseMatrix = glm::inverse(viewMatrix);

        projectionMatrix        = camera.getProjectionMatrix(viewWidth / viewHeight);
        projectionInverseMatrix = glm::inverse(projectionMatrix);

        projectionMatrix[1][1] *= -1;

        cameraPosition = camera.getPosition();

        nearPlane = camera.getNearPlane();
        farPlane  = camera.getFarPlane();

        frameTimeCounter = std::chrono::duration<float>(currentTime - startTime).count();

        ++frameCounter;
    }
};
