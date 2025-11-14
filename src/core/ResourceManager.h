#pragma once
#ifndef NOBLEENGINE_RESOURCEMANAGER_H
#define NOBLEENGINE_RESOURCEMANAGER_H

#include <string>

static const auto resourcesFile = std::string(RESOURCES_DIR);

static const std::string iconPath  = resourcesFile + "icon.png";

static const std::string shaderFilesPath  = resourcesFile + "shaders/spv/";
static const std::string textureFilesPath = resourcesFile + "textures/";
static const std::string modelFilesPath   = resourcesFile + "models/";

#endif //NOBLEENGINE_RESOURCEMANAGER_H
