#pragma once

#include "Mesh.h"

#include <filesystem>
#include <unordered_set>
#include <vector>

struct Model {
    std::string name = "Undefined_Model";

    std::vector<Mesh> meshes{};

    std::unordered_set<std::string> texturePaths{};

    void retrieveName(const std::string& path) {
        name = std::filesystem::path(path).stem().string();
    }

    void addMesh(const Mesh& mesh) {
        meshes.push_back(mesh);
    }
};
