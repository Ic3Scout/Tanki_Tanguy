#pragma once
#include "pch.h"
#include "Collider.h"


class SphereCollider : public Collider
{
public:
    SphereCollider(XMFLOAT3 _center, float _radius);
    
    bool IsColliding(Collider* pOther) override;
    bool SphereToSphere(SphereCollider* pOther);
    
protected:
    XMFLOAT3 m_center;
    float m_radius;
    
private:
    

};