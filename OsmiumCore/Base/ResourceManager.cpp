//
// Created by Shadow on 12/10/2024.
//

#include "ResourceManager.h"

#include <iostream>

namespace Resources {
    std::map<ResourceType,std::mutex> ResourceManager::resourcesMutexes;
    std::map<std::string, ResourceType> ResourceManager::resourceTable;

    void ResourceManager::Init() {
        resourcesMutexes.clear();
        resourceTable.clear();
        resourcesMutexes[ResourceType_Mesh];
        resourceTable["Mesh"] = ResourceType_Mesh;
    }

    /**
     * Get a mutex that can be used to safely interact with a resource type, Components and GameObject components should be able to provide it directly
     * @param resourceType the identifer of the resource type, can be obtained using ResourceManager::getResourceType, or the DefaultResourceType enum for built-in types
     * @return the mutex of the resource type
     */
    std::mutex & ResourceManager::getResourceMutex(unsigned short resourceType) {
        if(!resourcesMutexes.contains(resourceType)) {
            std::cout << "trying to get mutex for an unknownResourceType, returning default resource mutex" << std::endl;
            return resourcesMutexes[0];
        }
        return resourcesMutexes[resourceType];
    }

    /**
     * Converts a resource type name into its id, this is mainly intended for custom resource type, use a constexpr value of the DefaultresourceType enum for built-in types
     * @param type name of the type as defined in ResourcesType.conf
     * @return The Resource type as an unsigned short index
     */
    ResourceType ResourceManager::getResourceType(const std::string &type) {
        if(!resourceTable.contains(type)) {
            std::cout << "trying to get type for unknownResourceType, returning default resource type" << std::endl;
            return 0;
        }
        return resourceTable.at(type);
    }


    /**
     * Get a mutex that can be used to safely interact with a resource type.
     * @param type default resource type value
     * @return the mutex opf the resource type
     */
    std::mutex & ResourceManager::getResourceMutex(DefaultResourceType type) {
        return resourcesMutexes[type];//if the mutex doesn't exist it should be made by the map
    }
} // Resources