//
// Created by Basil on 2026-06-21.
//

#ifndef OSMIUM_GOC_COLLIDER_H
#define OSMIUM_GOC_COLLIDER_H
#include <memory>

#include "Base/GameObjectComponent.h"
#include "Collider.h"
#include "ResourceArray.h"


class GOC_Collider : public GameObjectComponent {

protected:
    static ResourceArray<GOC_Collider*,50> Colliders;
    std::shared_ptr<Collider> m_collider;
    unsigned int m_colliderHandle;
public:
    GOC_Collider(GameObject *parent, ShapeType type);
    GOC_Collider(GOC_Collider&& other) noexcept;
    GOC_Collider& operator=(GOC_Collider&& other) noexcept;

    ~GOC_Collider() override;

    struct contact {
        GOC_Collider *other;
        bool Refreshed = true;
    };
    std::vector<contact> NewContacts;
    std::vector<contact> OngoingContacts;

    void NotifyCollision(GOC_Collider * second);

    void OnContactStarts(const GOC_Collider * second);

    void OnContactOngoing(GOC_Collider * other);

    void OnContactOver();//must rely on internal data, as the other collider could be destroyed by then

    void ProcessContacts();

    static void CollisionStep();
    const std::string & Name() override = 0;
    const ShapeType Type;
};

class GOC_SphereCollider : public GOC_Collider {
public:
    const std::string name = "Sphere Collider";
    const std::string & Name() override{return name;}
    GOC_SphereCollider(GameObject *parent,float radius);
    explicit GOC_SphereCollider(GameObject* parent);

    GOC_SphereCollider(GOC_SphereCollider&& other) noexcept;
    GOC_SphereCollider& operator=(GOC_SphereCollider&& other) noexcept;
    float GetRadius() const;
    void SetRadius(float radius);
};

class GOC_BoxCollider : public GOC_Collider {
    public:
    const std::string name = "Box Collider";
    const std::string & Name() override{return name;}
    GOC_BoxCollider(GameObject *parent,float width,float height, float depth);
    explicit GOC_BoxCollider(GameObject *parent);
    GOC_BoxCollider(GOC_BoxCollider&& other) noexcept;
    GOC_BoxCollider& operator=(GOC_BoxCollider&& other) noexcept;
    glm::vec3 GetSize() const;
    void SetSize(const glm::vec3& size) const;
};

// class GOC_CapsuleCollider : public GOC_Collider {
//     public:
//     const std::string & Name() override{return "Capsule Collider";}
//     GOC_CapsuleCollider(GameObject *parent,float radius,float height);
// };
// class GOC_CylinderCollider : public GOC_Collider {
//     public:
//     const std::string & Name() override{return "Cylinder Collider";}
//     GOC_CylinderCollider(GameObject *parent,float radius,float height);
// };
#endif //OSMIUM_GOC_COLLIDER_H