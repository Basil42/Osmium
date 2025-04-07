//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef GAMEOBJECTCOMPONENT_H
#define GAMEOBJECTCOMPONENT_H
#include <string>


class GameObject;
typedef unsigned long GOC_Handle;
class GameObjectComponent {
protected:
    explicit GameObjectComponent(GameObject* parent);

private:
    GOC_Handle handle;
    GameObject* parent;

public:
    virtual const std::string&  Name() = 0;
    virtual void Update() {};//used for a more classical loop based update
    //all render updates should probably be static members
    virtual void RenderUpdate() {}
    [[nodiscard]] GameObject* GetGameObject() const {return parent;}
    [[nodiscard]] GOC_Handle GetObjectHandle() const {return handle;}
    GameObjectComponent() = delete;
    virtual ~GameObjectComponent() = default;
};



#endif //GAMEOBJECTCOMPONENT_H
