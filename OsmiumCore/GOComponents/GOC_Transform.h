//
// Created by nicolas.gerard on 2024-12-04.
//

#ifndef GOC_TRANSFORM_H
#define GOC_TRANSFORM_H
#include <vector>
#include <glm/glm.hpp>

#include "../Base/GameObjectComponent.h"
#include "../Helpers/Properties.h"


class GOC_Transform : public GameObjectComponent {
protected:
    ~GOC_Transform() override;

private:
    glm::mat4 model{};
    GOC_Handle parentTransform;
    std::vector<GOC_Handle> childrenTransforms;
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
    [[nodiscard]] glm::vec4 getRotation() const;
    void setRotation(const glm::vec4& newRotation);

    explicit GOC_Transform(GameObject* parent,const GOC_Transform *NewParentTransform);
    explicit GOC_Transform(GameObject* parent);//That should be essentially automated
};



#endif //GOC_TRANSFORM_H
