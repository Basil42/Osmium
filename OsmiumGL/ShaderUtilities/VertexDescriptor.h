//
// Created by nicolas.gerard on 2025-01-21.
//

#ifndef VERTEXDESCRIPTOR_H
#define VERTEXDESCRIPTOR_H

enum DefaultVertexAttributeFlags : unsigned int {
    POSITION = 1 << 0,
    NORMAL = 1 << 1,
    TEXCOORD0 = 1<<2,
    //not explicitely supported by the obj file format
    TANGENT = 1<<3,
    BITANGENT0 = 1<<4,
    JOINTID = 1<<5,
    MAX_VERTEX_ATTRIBUTE_FLAGS = JOINTID
};

constexpr DefaultVertexAttributeFlags operator|(DefaultVertexAttributeFlags a, DefaultVertexAttributeFlags b) {
    return static_cast<DefaultVertexAttributeFlags>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
}

inline DefaultVertexAttributeFlags& operator|=(DefaultVertexAttributeFlags& a, DefaultVertexAttributeFlags b) {
    a = a | b;
    return a;
}

struct VertexBufferDescriptor {
    unsigned int AttributeStride;
    void* data;//pre offset pointer to source data
    DefaultVertexAttributeFlags attribute;//should be a single flag
    unsigned int customAttribute;//should only be set if attribute is 0
    //vertex count is the same for every buffer allocated in a single mesh load, I'll ignore it for now
};
#endif //VERTEXDESCRIPTOR_H
