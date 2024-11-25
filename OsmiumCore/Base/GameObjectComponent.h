//
// Created by nicolas.gerard on 2024-11-25.
//

#ifndef GAMEOBJECTCOMPONENT_H
#define GAMEOBJECTCOMPONENT_H


class GameObject;

class GameObjectComponent {
    GameObject* parent;
    virtual void Update(){}//used for a more classical loop based update

};



#endif //GAMEOBJECTCOMPONENT_H
