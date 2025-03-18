//
// Created by nicolas.gerard on 2025-03-18.
//

#include "../include/Serialization.h"
#include <MeshData.h>

/**
 * Import a supported mesh file into a format that can be quickly loaded into OsmiumGL using the source file associated .meta file.
 * Updates or create a default .meta file on successful import and save the imported data into the resource folder.
 * @param filePath source file to import
 * @param destination folder to import to, usually the resource folder
 * @return success
 */
bool Serialization::ImportMeshAsset(const std::filesystem::path &filePath, const std::filesystem::path &destination) {
    MeshData data{};
    if (!ImportMeshAsset(filePath, data))
        return false;

    //generate GUID(find a portable solution to this)
    //save in resource folder as GUID.mesh

}

/**
 * Import a supported mesh file to a struct that can be loaded into OsmiumGL, useful for runtime imports
 * @param filePath source file to import
 * @param meshData raw input of the import, useful for runtime import of supported types
 * @return success
 */
bool Serialization::ImportMeshAsset(const std::filesystem::path &filePath, MeshData &meshData) {
}

bool Serialization::DeserializeMeshAsset(const std::filesystem::path &filePath, MeshData &data) {
}
