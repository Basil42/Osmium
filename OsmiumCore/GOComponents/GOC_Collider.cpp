//
// Created by Basil on 2026-06-21.
//

#include "GOC_Collider.h"

#include "CoreUtils.h"
#include "GOC_Transform.h"
#include "Intersection.h"
#include "Base/GameInstance.h"

ResourceArray<GOC_Collider*,50> GOC_Collider::Colliders;
GOC_Collider::GOC_Collider(GameObject *parent, const ShapeType type) : GameObjectComponent(parent), Type(type){
    m_colliderHandle = 51;//setting it to an invalid handle to mark it as uninitialized
}

GOC_Collider::GOC_Collider(GOC_Collider &&other) noexcept : GameObjectComponent(other.GetGameObject()), Type(other.Type) {//does not garanteed that the parent know about the copy, but these should not be copied anyway
    m_colliderHandle = other.m_colliderHandle;
    m_collider = other.m_collider;
}

GOC_Collider & GOC_Collider::operator=(GOC_Collider &&other) noexcept {
    if (this != &other) {
        m_colliderHandle = other.m_colliderHandle;
        m_collider = other.m_collider;
        parent = other.GetGameObject();

    }
    return *this;
}

GOC_Collider::~GOC_Collider() {
    if (m_colliderHandle == 51)return;//we weren't in the Collider collection
    Colliders.Remove(m_colliderHandle);
}

void GOC_Collider::NotifyCollision(GOC_Collider *second) {
    for (auto& contact : OngoingContacts) {
        if (contact.other == second) {
            contact.Refreshed = true;
            return;
        }
    }
    NewContacts.emplace_back(second,true);
}

void GOC_Collider::OnContactStarts(const GOC_Collider *second) {
    std::cout << GetGameObject()->Name << " contact started with " << second->GetGameObject()->Name << std::endl;
    //can add various callbacks here
}

void GOC_Collider::OnContactOngoing(GOC_Collider *other) {
    if (other->m_colliderHandle == 51) {//mostly here to check that the pointer stays valid
        assert(false);
    }
}

void GOC_Collider::OnContactOver() {
    std::cout << GetGameObject()->Name << " contact over with " << GetGameObject()->Name << std::endl;
}

void GOC_Collider::ProcessContacts() {
    //dealing with new contacts
    while (!NewContacts.empty()) {
        OnContactStarts(NewContacts.back().other);
        OngoingContacts.emplace_back(NewContacts.back());
        NewContacts.pop_back();
    }
    //process contacts (this happens on the frame the contact starts too)
    for (unsigned int i = 0; i < OngoingContacts.size(); i++) {
        if (OngoingContacts[i].Refreshed) {
            OnContactOngoing(OngoingContacts[i].other);
            OngoingContacts[i].Refreshed = false;
        }
        else {
            OnContactOver();//I don't think contact over should be allowed to access the other pointer, it could be invalid at this point
            OngoingContacts.erase(OngoingContacts.begin() + i--);
        }
    }
}

void GOC_Collider::CollisionStep() {
    std::vector<std::pair<GOC_Collider*,GOC_Collider*>> ActiveCollisions;//really bad but I don't want to waste time doing something efficient for this
    //intersection check
    for (auto i = Colliders.begin(); i != Colliders.end(); ++i) {
        for (auto j = i+1; j != Colliders.end(); ++j) {
            if (Intersection::CheckIntersection(*(*i)->m_collider,*(*j)->m_collider)) {
                ActiveCollisions.emplace_back(*i,*j);
            }
        }
    }
    //refresh
    for (std::pair<GOC_Collider *, GOC_Collider *> &collision: ActiveCollisions) {
        collision.first->NotifyCollision(collision.second);
        collision.second->NotifyCollision(collision.first);
    }
    //responses
    //they can't invalidate iterator, probable having responses in the operation queue
    for (const auto& collider : Colliders) {
        collider->ProcessContacts();
    }

}



GOC_SphereCollider::GOC_SphereCollider(GameObject *parent, float radius) : GOC_Collider(parent, ShapeType::Sphere) {
    const auto transform = parent->GetComponent<GOC_Transform>();
    ASSERT(transform != nullptr, "Colliders need to have a GOC_Transform");
    m_collider = std::make_shared<SphereCollider>(transform->getTransformMatrixConstRef(),radius);
    m_colliderHandle = Colliders.Add(this);

}

GOC_SphereCollider::GOC_SphereCollider(GameObject *parent) : GOC_SphereCollider(parent,1.0f){

}

GOC_SphereCollider::GOC_SphereCollider(GOC_SphereCollider &&other) noexcept : GOC_Collider(other.GetGameObject(),Sphere) {
    m_colliderHandle = other.m_colliderHandle;
    m_collider = other.m_collider;
}

GOC_SphereCollider & GOC_SphereCollider::operator=(GOC_SphereCollider &&other) noexcept {
    m_colliderHandle = other.m_colliderHandle;
    m_collider = other.m_collider;
    parent = other.parent;
    return *this;
}

float GOC_SphereCollider::GetRadius() const {
    return dynamic_cast<SphereCollider*>(m_collider.get())->Radius;
}

void GOC_SphereCollider::SetRadius(float radius) {
    dynamic_cast<SphereCollider*>(m_collider.get())->Radius = radius;
}

GOC_BoxCollider::GOC_BoxCollider(GameObject *parent, const float width, const float height, const float depth) : GOC_Collider(parent, ShapeType::Box) {
    const auto transform = parent->GetComponent<GOC_Transform>();
    ASSERT(transform != nullptr, "Colliders need to have a GOC_Transform");
    m_collider = std::make_shared<BoxCollider>(transform->getTransformMatrixConstRef(),glm::vec3(width,height,depth));
    m_colliderHandle = Colliders.Add(this);
}

GOC_BoxCollider::GOC_BoxCollider(GameObject *parent) : GOC_BoxCollider(parent,1.0f,1.0f,1.0f){
}

GOC_BoxCollider::GOC_BoxCollider(GOC_BoxCollider &&other) noexcept :GOC_Collider(other.GetGameObject(),Box) {
    m_colliderHandle = other.m_colliderHandle;
    m_collider = other.m_collider;
}

GOC_BoxCollider & GOC_BoxCollider::operator=(GOC_BoxCollider &&other) noexcept {
    m_colliderHandle = other.m_colliderHandle;
    m_collider = other.m_collider;
    parent = other.parent;
    return *this;
}

glm::vec3 GOC_BoxCollider::GetSize() const {
    return dynamic_cast<BoxCollider*>(m_collider.get())->size;
}

void GOC_BoxCollider::SetSize(const glm::vec3 &size) const {
    dynamic_cast<BoxCollider*>(m_collider.get())->size = size;
}
