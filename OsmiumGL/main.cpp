

// ReSharper disable once CppClassNeedsConstructorBecauseOfUninitializedMember


#include <iostream>

#include "Core.h"

int main() {
    try {
        OsmiumGLInstance app;
        app.run();
    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
