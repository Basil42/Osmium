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
  inline auto ClosestPointOnAabb(glm::vec3 boxsize, glm::vec3 position) -> glm::vec3 {
    return glm::clamp(position,-boxsize/2.0f , boxsize/2.0f);
  }

  struct Simplex {
    std::array<glm::vec3, 4> vertices{};
    unsigned short count = 0;
    void addVertex(const glm::vec3 vertex) {
      vertices[count++] = vertex;
    }

  };
}

#endif //SHAPES_H
