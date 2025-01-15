//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H
#include <map>
#include <typeindex>
#include <vector>

class GameObjectComponent;
/**
 * Non ecs entities, for convenience and getting started
 */
class GameObject {
public:
    bool RemoveComponent(GameObjectComponent* component);
    void UpdateComponents();

    template<typename T,
       typename = std::enable_if_t<std::derived_from<T, GameObjectComponent>>>
    T* Addcomponent(){
        T* comp = new T(this);
        this->components.insert(std::make_pair(std::type_index(typeid(T)), comp));
        return comp;
    }

    void Destroy();

    template<typename T,
           typename = std::enable_if_t<std::is_base_of_v<GameObjectComponent, T>>>
    T* GetComponent(){
        auto it = components.find(std::type_index(typeid(T)));
        if (it == components.end()) return nullptr;
        return dynamic_cast<T*>(it->second);
    }

private:
    std::multimap<std::type_index,GameObjectComponent*> components;
};




#endif //GAMEOBJECT_H
