//
// Created by nicolas.gerard on 2025-02-17.
//

#ifndef SHAPES_H
#define SHAPES_H
#include <array>
#include <iterator>

enum ShapeType{
  Sphere,
  Box,
  Capsule,
  Cylinder,
  };

namespace Shapes {
  constexpr std::array<ShapeType,4> ShapeTypes = {Sphere,Box,Capsule,Cylinder};
  constexpr unsigned int NumShapeTypes = std::size(ShapeTypes);
  inline glm::vec3 ClosestPointOnAabb(glm::vec3 boxsize, glm::vec3 position) {
    return glm::clamp(position,-boxsize/2.0f , boxsize/2.0f);
  }
}

#endif //SHAPES_H
