

// ReSharper disable once CppClassNeedsConstructorBecauseOfUninitializedMember


#include <iostream>

#include "Core/Core.h"

int main() {
    try {
        HelloTriangleApplication app;
        app.run();
    }catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
