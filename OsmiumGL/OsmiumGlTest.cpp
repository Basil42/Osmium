#include <iostream>
#include <ostream>
#include <stdexcept>

#include "OsmiumGL_API.h"
//
// Created by nicolas.gerard on 2024-12-02.
//
int main(int argc, char *argv[]) {
    try {
        OsmiumGL::Init();

    } catch (std::runtime_error) {
        std::cout << "An exception occured!" << std::endl;
    }
    return 0;
}