#pragma once
#ifndef NOBLEENGINE_MODEL_H
#define NOBLEENGINE_MODEL_H

#include "Mesh.h"

#include <filesystem>
#include <vector>

struct Model {
    std::string name = "Undefined_Model";

    std::vector<Mesh> meshes{};

    std::vector<std::string> texturePaths{};

    void retrieveName(const std::string& path) {
        name = std::filesystem::path(path).stem().string();
    }

    void addMesh(const Mesh& mesh) {
        meshes.push_back(mesh);
    }
};

#endif // NOBLEENGINE_MODEL_H
