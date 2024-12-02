//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef OSMIUMGL_API_H
#define OSMIUMGL_API_H
class  OsmiumGL {
public:
    static void Init();
    static void Shutdown();

    typedef unsigned long MeshHandle;
    MeshHandle LoadMesh(const char* filename);
};


#endif //OSMIUMGL_API_H
