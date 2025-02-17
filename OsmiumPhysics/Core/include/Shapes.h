//
// Created by nicolas.gerard on 2025-02-17.
//

#ifndef SHAPES_H
#define SHAPES_H
#include <iterator>

enum ShapeType{
  Sphere,
  Box,
  Capsule,
  Cylinder,
  };

constexpr ShapeType ShapeTypes[] = {Sphere,Box,Capsule,Cylinder};
constexpr unsigned int NumShapeTypes = std::size(ShapeTypes);
#endif //SHAPES_H
