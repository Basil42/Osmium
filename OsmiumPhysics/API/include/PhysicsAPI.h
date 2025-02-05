//
// Created by Shadow on 2/4/2025.
//

#ifndef PHYSICSAPI_H
#define PHYSICSAPI_H
class PhysicsScene;
class PhysicsAPI {
    static PhysicsScene* scene;
    void Simulate(double deltaTime);
};
#endif //PHYSICSAPI_H
