//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef MEMCHECK_H
#define MEMCHECK_H
#include <iostream>
#include <ostream>
#if WIN32
#include <windows.h>

static bool CanAllocate(size_t size) {
    MEMORYSTATUS statex;
    statex.dwLength = sizeof(statex);
    GlobalMemoryStatus(&statex);
    // std::cout << statex.ullTotalPhys / 1024 / 1024 << std::endl;
    // std::cout << statex.ullAvailPhys / 1024 / 1024 << std::endl;
    return statex.ullAvailPhys >= size;//probably needs some kind of overhead
}
#endif

#if __linux


static bool CanAllocate(size_t size) {
    return true;//getting this information precisely is annoying on linux, I'll solve it if I need todefault
}
#endif
#endif //MEMCHECK_H
