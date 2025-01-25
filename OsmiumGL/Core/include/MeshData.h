//
// Created by Shadow on 1/19/2025.
//
#ifndef MESHDATA_H
#define MESHDATA_H
#include "VertexDescriptor.h"
#include <map>

struct MeshData {
  DefaultVertexAttributeFlags attributeFlags;
  unsigned int customAttributesFlags;//user defined custom attributes like color
  unsigned int numVertices;
  unsigned int numIndices;
  //buffers data, could probably be a lot faster
  std::map<DefaultVertexAttributeFlags, std::pair<VkBuffer,VmaAllocation>> VertexAttributeBuffers;
  VkBuffer indexBuffer;
  VmaAllocation IndexBufferAlloc;
};
#endif //MESHDATA_H
