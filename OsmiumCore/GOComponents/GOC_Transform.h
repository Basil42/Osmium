//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GOC_TRANSFORM_H
#define GOC_TRANSFORM_H
#include <vector>
#include <glm/glm.hpp>
#include <glm/detail/type_quat.hpp>

#include "../Base/GameObject.h"
#include "../Base/GameObjectComponent.h"
#include "../Base/GameObjectCreation.h"
#include "../Helpers/Properties.h"
struct GOC_TransformCreateData : GameObjectComponentCreateInfo{//afraid this solution will quickly slow down compilation
    GameObjectHandle parent;

};

class GOC_Transform : public GameObjectComponent {

private:
    glm::mat4 model = glm::mat4(1.0f);
    GOC_Handle parentTransform;
    std::vector<GOC_Handle> childrenTransforms;
    glm::vec3 cachedScale = glm::vec3(1.0f);
    glm::quat cachedRotation = glm::quat();
    glm::vec4 cachedPerspective;
    glm::vec3 cachedSkew;
    [[nodiscard]] GOC_Handle getParentTransform() const { return parentTransform; }
    void setParent(const GOC_Handle &newParentTransform){ parentTransform = newParentTransform; }
    //glm::vec3 getRootPosition();
    //glm::vec3 getRootScale();
    //glm::vec4 getRootRotation();

public:
    [[nodiscard]] glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& newPosition);
    [[nodiscard]] glm::vec3 getScale() const;
    void setScale(const glm::vec3& newScale);
    [[nodiscard]] glm::quat getRotation() const;
    void setRotation(const glm::quat& newRotation);
    //[[nodiscard]] glm::vec3 getEulerRotation() const;
    [[nodiscard]] glm::mat4 getTransformMatrix() const;
    void getTransformDecomposed(glm::vec3& translation,glm::quat& rotation,glm::vec3& scale)const;
    void getTransformDecomposed(glm::vec3& translation,glm::quat& rotation,glm::vec3& scale,glm::vec3 &skew, glm::vec4& perspective)const;

    void SetTransformMatrix(glm::mat4 mat);
    void SetTransformMatrix(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale, const glm::vec3 &skew, const glm::vec4& perspective);
    explicit GOC_Transform(GameObject* parent,const GOC_Transform *NewParentTransform);
    explicit GOC_Transform(GameObject* parent);//That should be essentially automated
    ~GOC_Transform() override;

    const std::string name = "Transform";
    const std::string &Name() override {
        return name;
    }
};



#endif //GOC_TRANSFORM_H
