//
// Created by Shadow on 1/26/2025.
//

#ifndef GOC_CAMERA_H
#define GOC_CAMERA_H
#include <glm/fwd.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Base/GameObjectComponent.h"


class GOC_Transform;

enum rotationMode {
    ROTATION_MODE_TRANSFORM,//target value will update to be towards the forward vector of the transform
    ROTATION_MODE_TARGET //transform will be updated to align the forward vector of the transform with the camera target
};

class GOC_Camera : public GameObjectComponent {
    GOC_Transform* transform;//contains the eye position and up
    //field of view
    glm::vec3 target = glm::vec3();//in world position, should be either controlled or controlling the rotation of the camera transform
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    rotationMode rotationMode = ROTATION_MODE_TRANSFORM;
    void Update() override;
public:
    explicit GOC_Camera(GameObject* parent);
    ~GOC_Camera() override = default;

    void RenderUpdate() override;

    [[nodiscard]] glm::mat4 GetViewMatrix() const;
    [[nodiscard]] glm::mat4 GetTransform() const;
    void SetTransform(const glm::mat4& transform);

    const std::string name = "Camera";
    float verticalFoV = 90.0f; //in degrees, GL will find the proj matrix on top of a pass that uses this camera
    const std::string & Name() override {
        return name;
    };

};



#endif //GOC_CAMERA_H
