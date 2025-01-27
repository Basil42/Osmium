//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef GAMEOBJECTCOMPONENT_H
#define GAMEOBJECTCOMPONENT_H
#include <cstdint>


class GameObject;
typedef unsigned long GOC_Handle;
class GameObjectComponent {
protected:
    explicit GameObjectComponent(GameObject* parent);

private:
    GOC_Handle handle;
    GameObject* parent;
    virtual void Update() {};//used for a more classical loop based update


public:
    virtual void RenderUpdate() {}
    [[nodiscard]] GameObject* GetGameObject() const {return parent;}
    [[nodiscard]] GOC_Handle GetHandle() const {return handle;}
    GameObjectComponent() = delete;
    virtual ~GameObjectComponent() = default;
};



#endif //GAMEOBJECTCOMPONENT_H
