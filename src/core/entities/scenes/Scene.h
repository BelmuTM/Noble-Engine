#pragma once
#ifndef NOBLEENGINE_SCENE_H
#define NOBLEENGINE_SCENE_H

#include "core/entities/objects/Object.h"

class Scene {
public:
    Scene()  = default;
    ~Scene() = default;

    void addObject(
        const std::string& modelPath,
        const glm::vec3    position,
        const glm::vec3    rotation,
        const glm::vec3    scale
    ) {
        _objects.emplace_back(modelPath, position, rotation, scale);
    }

    [[nodiscard]] const std::vector<ObjectDescriptor>& getObjects() const noexcept { return _objects; }

private:
    std::vector<ObjectDescriptor> _objects{};
};

#endif // NOBLEENGINE_SCENE_H
