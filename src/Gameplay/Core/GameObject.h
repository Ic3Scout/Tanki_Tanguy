#pragma once
#include "pch.h"

class GameObject
{
public:
    GameObject(int isServer);
    virtual ~GameObject();
    
    virtual void Update() = 0;
    virtual void Dead(float _dt) = 0;
    
    void Move(float _dir);
    void Turn(float _dir);

    std::string const& GetTag() { return m_tag; }
    XMFLOAT3& GetDir();
    XMFLOAT3& GetPos();
    bool IsServer() { return m_isServer; }
    bool IsNeedToBeDestroyed() { return m_needToBeDestroyed; }
    void SetNeedToBeDestroyed() { m_needToBeDestroyed = true; }
    
    cpu_transform m_transform;
    cpu_entity* m_pEntity = nullptr;
    
protected:
    cpu_mesh m_mesh;
    cpu_material m_material;
    XMFLOAT3 m_color;
    
    bool m_isServer = false;
    bool m_needToBeDestroyed = false;
    
    std::string m_tag;
    
    float m_angle = XM_PI;
    float m_speed = 0.0f;
private:
    
};

