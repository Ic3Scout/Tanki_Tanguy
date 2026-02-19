#include "SphereCollider.h"
#include "GameObject.h"
#include "Utils.hpp"

using namespace Utils;

SphereCollider::SphereCollider(XMFLOAT3 _center, float _radius) : m_center(_center), m_radius(_radius)
{}

////////////////////////////////////////////////////////////
/// @brief Chooses the correspond collision detection method
////////////////////////////////////////////////////////////
bool SphereCollider::IsColliding(Collider* pOther)
{
    SphereCollider* oS = dynamic_cast<SphereCollider*>(pOther);
    if (oS != nullptr) return SphereToSphere(oS);

    //TODO: BoxCollider
    return false;
}

/////////////////////////////////////////////////////
/// @brief Sphere to Sphere collision detection
/////////////////////////////////////////////////////
bool SphereCollider::SphereToSphere(SphereCollider* pOther)
{
    SphereCollider* o = pOther;
    if (o == nullptr) return false;

    // Adapts the calculations to the Physics Components using the next frame
    XMFLOAT3 pos = m_center;
    XMFLOAT3 oPos = o->m_center;

    // Calculates the distance between the 2 spheres
    float distance = sqrt(pow(pos.x - oPos.x, 2) + pow(pos.y - oPos.y, 2) + pow(pos.z - oPos.z, 2));

    return (distance < m_radius + o->m_radius);
}

