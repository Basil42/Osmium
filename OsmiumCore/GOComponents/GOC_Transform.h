//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GOC_TRANSFORM_H
#define GOC_TRANSFORM_H
#include <vector>
#include <glm/glm.hpp>

#include "../Base/GameObjectComponent.h"
#include "../Helpers/Properties.h"


class GOC_Transform : GameObjectComponent {
    glm::mat4 model;
    GOC_Handle parentTransform;
    std::vector<GOC_Handle> childrenTransforms;
    [[nodiscard]] GOC_Handle getParentTransform() const { return parentTransform; }
    void setParent(const GOC_Handle &newParentTransform){ parentTransform = newParentTransform; }
    //glm::vec3 getRootPosition();
    //glm::vec3 getRootScale();
    //glm::vec4 getRootRotation();
    [[nodiscard]] glm::vec3 getPosition() const;
    void setPosition(const glm::vec3& newPosition);
    [[nodiscard]] glm::vec3 getScale() const;
    void setScale(const glm::vec3& newScale);
    [[nodiscard]] glm::vec4 getRotation() const;
    void setRotation(const glm::vec4& newRotation);

public:
    explicit GOC_Transform(GOC_Transform *parent);
    //you cannot pass these around by reference, they are just a convenient wrapper around getter and setters
    Property<GOC_Transform, glm::vec3, &getPosition, &setPosition> Position;
    Property<GOC_Transform, glm::vec3, &getScale,&setScale> Scale;
    Property<GOC_Transform,glm::vec4,&getRotation, &setRotation> Rotation;
    Property<GOC_Transform, GOC_Handle,&getParentTransform,&setParent> Parent;
};



#endif //GOC_TRANSFORM_H
