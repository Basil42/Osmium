//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <functional>
#include <map>
#include <string>
#include <typeindex>
#include <vector>

#include "GameObjectComponent.h"

typedef unsigned int GameObjectHandle;
/**
 * Non ecs entities, for convenience and getting started
 */
class GameObject {
public:
    std::string Name = "gameobject";//This is solely for human users, I might want to cut it from release builds, or confine it to UI data
    GameObjectHandle Handle;

    bool RemoveComponent(const GameObjectComponent* component);

    template<typename T,
       typename = std::enable_if_t<std::derived_from<T, GameObjectComponent>>>
    int RemoveComponents() {//removes all component of the provided type
        auto it = components.lower_bound(std::type_index(typeid(T)));
        while (it != components.upper_bound(std::type_index(typeid(T)))) {
            delete it->second;
        }
        return components.erase(std::type_index(typeid(T)));
    }
    void UpdateComponents();

    template<typename T,
       typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent, T>>>
    T* Addcomponent(std::function<void(T*)> callback){
        T* component = Addcomponent<T>();
        callback(component);
        return component;
        // T* comp = new T(this);//Instead of this random allocation, each componenet could have a container despite being used like a unity game object
        // this->components.insert(std::make_pair(std::type_index(typeid(T)), comp));
        // return comp;
    }
    template<typename T,
       typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent,T>>>
    T* Addcomponent(){//that might be safe to call mid sim, the map elements are memory stable
        T* component = new T(this);
        auto it = components.emplace(std::type_index(typeid(T)),component);
        return component;
        // T* comp = new T(this);//Instead of this random allocation, each componenet could have a container despite being used like a unity game object
        // //this->components.insert(std::make_pair(std::type_index(typeid(T)), comp));
        // return comp;
    }


    template<typename T,
           typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent, T>>>
    T* GetComponent() const {
        auto it = components.find(std::type_index(typeid(T)));
        if (it == components.end()) return nullptr;
        return dynamic_cast<T*>(it->second);
    }

    ~GameObject();//Bad, I don't want this to be public but it is easier for now
    const std::multimap<std::type_index, GameObjectComponent *> &GetComponents() const;

private:
    std::multimap<std::type_index,GameObjectComponent*> components;//reference to these pairs(and components) should be stable
};




#endif //GAMEOBJECT_H
