//
// Created by nicolas.gerard on 2024-12-02.
//

#ifndef MEMCHECK_H
#define MEMCHECK_H
#include <iostream>
#include <ostream>
#include <windows.h>

static bool CanAllocate(size_t size) {
    MEMORYSTATUSEX statex; statex.dwLength = sizeof(statex);
    GlobalMemoryStatusEx(&statex);
    std::cout << statex.ullTotalPhys / 1024 / 1024 << std::endl;
    std::cout << statex.ullAvailPhys / 1024 / 1024 << std::endl;
    return statex.ullAvailPhys >= size;//probably needs some kind of overhead
}
#endif //MEMCHECK_H
