//
// Created by nicolas.gerard on 2025-04-10.
//

#ifndef PARSEDMATERIAL_H
#define PARSEDMATERIAL_H
#include <span>
#include <filesystem>


class ParsedMaterial {
    ParsedMaterial(std::span<std::filesystem::path> shaders);//takes a span of shader files (alternating between vertex and fragment) and build a pipeline per pair
    ~ParsedMaterial();
};



#endif //PARSEDMATERIAL_H
