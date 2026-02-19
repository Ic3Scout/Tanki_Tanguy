#include "GameObject.h"
#include "GameManager.h"

GameObject::GameObject(int isServer)
{
    m_isServer = isServer;
    if (m_isServer == false)
    {
        m_pEntity = cpuEngine.CreateEntity();
    }
}

GameObject::~GameObject()
{
    if (!m_isServer) 
        cpuEngine.Release(m_pEntity);
}

void GameObject::Update()
{
}

void GameObject::Move(float _dir)
{
    if (m_isServer)
        m_transform.Move(_dir * GameManager::Get().GetDeltaTime() * m_speed);
    else
        m_pEntity->transform.Move(_dir * cpuTime.delta * m_speed);
}

void GameObject::Turn(float _dir)
{
    if (m_isServer)
        m_transform.AddYPR(_dir * GameManager::Get().GetDeltaTime());
    else
        m_pEntity->transform.AddYPR(_dir * cpuTime.delta);
}

XMFLOAT3& GameObject::GetDir()
{
    if (m_isServer)
        return m_transform.dir;

    return m_pEntity->transform.pos;
}

XMFLOAT3& GameObject::GetPos()
{
    if (m_isServer)
        return m_transform.pos;

    return m_pEntity->transform.pos;
}
