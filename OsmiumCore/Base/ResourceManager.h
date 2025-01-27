//
// Created by Shadow on 12/10/2024.
//

#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H
#include <map>
#include <mutex>
#include <string>

namespace Resources {
    /**
    * All defined entries will be attributed a mutex, user should be able to define their own
    */
    typedef unsigned short ResourceType;
    enum DefaultResourceType {
        ResourceType_Other = 0,
        ResourceType_Mesh = 1,
        ResourceType_Texture = 2,
        ResourceType_Entity = 3,
        ResourceType_Component = 4,
        ResourceType_GameObject = 5,
        ResourceType_GameObjectComponent = 6,
        ResourceType_Scene = 7

    };
    /**
 * Provides mutexes to lockj various resources for synchronisation purposes
 */
class ResourceManager {
    static std::map<ResourceType,std::mutex> resourcesMutexes;
    /**
     * for users and to have human readable
     */
    static std::map<std::string, ResourceType> resourceTable;
public:
    ResourceManager() = delete;
    static void Init();
    static std::mutex& getResourceMutex(ResourceType);
    static ResourceType getResourceType(const std::string &);
    static std::mutex& getResourceMutex(DefaultResourceType type);
};


} // Resources

#endif //RESOURCEMANAGER_H
