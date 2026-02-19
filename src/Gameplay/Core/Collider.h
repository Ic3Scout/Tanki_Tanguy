#pragma once
#include "pch.h"

class GameManager;
class SphereCollider;
class GameObject;

class Collider 
{
public:
    Collider() = default;
    virtual ~Collider() { m_collidingEntities.clear(); }

    GameObject* GetOwner() { return m_pOwner; }

    virtual bool IsColliding(Collider* pOther) = 0;
    
    virtual void OnCollisionEnter(Collider* pOther);
    virtual void OnCollisionStay (Collider* pOther);
    virtual void OnCollisionExit (Collider* pOther);
    
protected:
    GameObject* m_pOwner = nullptr;
    bool m_isActiveCollider = true;
    
    bool m_isTrigger = false;
    std::vector<Collider*> m_collidingEntities;
    bool IsAlreadyColliding(Collider* pOther);

private:
    
    friend class GameManager;
};