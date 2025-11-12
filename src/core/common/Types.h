#pragma once
#ifndef NOBLEENGINE_TYPES_H
#define NOBLEENGINE_TYPES_H

#include <memory>
#include <vector>

class Object;

using objects_vector = std::vector<std::unique_ptr<Object>>;

#endif // NOBLEENGINE_TYPES_H
