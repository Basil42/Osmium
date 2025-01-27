//
// Created by Shadow on 1/16/2025.
//

#ifndef RENDEREDOBJECT_H
#define RENDEREDOBJECT_H
#include <vector>



namespace std {
    enum class byte : unsigned char;
}

typedef unsigned long MaterialHandle;
typedef unsigned long MatInstanceHandle;
typedef unsigned long MeshHandle;
typedef unsigned long ConstantsHandle;
struct RenderedObject {//Struct entries are ordered like they are in the render tree
    MaterialHandle material;//this is mostly the pipeline
    MatInstanceHandle matInstance;//uniform set
    MeshHandle mesh;
};

// struct RenderedEntity {
//
// };



#endif //RENDEREDOBJECT_H
