//
// Created by Shadow on 12/10/2024.
//

#include "ResourceManager.h"

#include <iostream>

namespace Resources {
    std::map<unsigned short,std::pair<std::string,std::mutex>> ResourceManager::resourcesMutexes;
    std::map<std::string, ResourceType> ResourceManager::resourceTable;
    /**
     * Get a mutex that can be used to safely interact with a resource type, Components and GameObject components should be able to provide it directly
     * @param resourceType the identifer of the resource type, can be obtained using ResourceManager::getResourceType, or the DefaultResourceType enum for built-in types
     * @return the mutex of the resource type
     */
    std::mutex & ResourceManager::getResourceMutex(unsigned short resourceType) {
        if(!resourcesMutexes.contains(resourceType)) {
            std::cout << "trying to get mutex for an unknownResourceType, returning default resource mutex" << std::endl;
            return resourcesMutexes.at(0).second;
        }
        return resourcesMutexes.at(resourceType).second;
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
        return resourcesMutexes.at(type).second;//not doing safety check as this should be garanteed to work
    }
} // Resources