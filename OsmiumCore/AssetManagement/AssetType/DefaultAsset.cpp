//
// Created by Shadow on 1/15/2025.
//

#include "DefaultAsset.h"

#include <iostream>

void DefaultAsset::Load_Impl() {
    std::cout << "trying to load an unsuported asset" << std::endl;
}

void DefaultAsset::Unload_Impl(bool immediate) {
    std::cout << "trying to unload an unsuported asset" << std::endl;
}

std::mutex & DefaultAsset::GetRessourceMutex() {
    throw std::runtime_error("Trying to get resource mutex for a non supported ressource type, this is not supported");
}

DefaultAsset::DefaultAsset(const xg::Guid& guid) : Asset(guid) {
}

DefaultAsset::DefaultAsset() : Asset(xg::Guid()){
}
