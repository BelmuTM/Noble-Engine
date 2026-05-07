#pragma once

#include "Mesh.h"

#include <filesystem>
#include <unordered_set>
#include <vector>

struct Model {
    std::string path;
    std::string name = "Undefined_Model";

    std::vector<Mesh> meshes{};

    std::unordered_set<std::string> texturePaths{};

    void retrieveName(const std::string& filePath) {
        name = std::filesystem::path(filePath).stem().string();
    }

    void addMesh(const Mesh& mesh) {
        meshes.push_back(mesh);
    }
};
