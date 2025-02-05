//
// Created by Shadow on 2/4/2025.
//

#ifndef PHYSICSSCENE_H
#define PHYSICSSCENE_H


class PhysicsScene {
    public:
    PhysicsScene();
    void Simulate(double deltaTime);

    void ApplyForces(double deltaTime);
    void IntersectionsCheck();
    void HandleCollisions();
    void UpdateScene(double deltaTime);
    //wrap up step ?
};



#endif //PHYSICSSCENE_H
