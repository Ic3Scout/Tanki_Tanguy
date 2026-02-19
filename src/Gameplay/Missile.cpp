#include "Missile.h"

#include "Player.h"
#include "Core/Score.hpp"
#include "Core/GameManager.h"

Missile::Missile(bool _isServer) : GameObject(_isServer), SphereCollider(XMFLOAT3(1.0f, 1.0f, 1.0f), 1.0f), m_pEmitter(nullptr)
{
    m_firstPos = XMFLOAT3(1.0f, 1.0f, 1.0f);
    if (_isServer)
    {
        m_transform.sca = XMFLOAT3(0.4f, 0.4f, 0.4f);
    }
    else
    {
        m_mesh.CreateSphere();
        m_material.color = cpu::ToColor(87, 87, 87);

        m_pEntity = cpuEngine.CreateEntity();
        m_pEntity->pMesh = &m_mesh;
        m_pEntity->pMaterial = &m_material;
        m_pEntity->transform.sca = XMFLOAT3(0.4f, 0.4f, 0.4f);

        m_pEmitter = cpuEngine.CreateParticleEmitter();
        m_pEmitter->density = 100.0f;
        m_pEmitter->colorMin = cpu::ToColor(255, 0, 0);
        m_pEmitter->colorMax = cpu::ToColor(255, 128, 0);
    }
}

Missile::~Missile()
{
    cpuEngine.Release(m_pEmitter);
}

void Missile::Init(cpu_transform& _transform, Player& _player)
{
    m_tag = "Missile";
    m_pOwnerPlayer = &_player;
    m_speed = 10.0f;
    m_firstPos = _transform.pos;
    XMFLOAT3 forward = _transform.dir;

    if (m_isServer)
    {
        m_transform.pos = _transform.pos;
        m_transform.pos.x += forward.x * 2.0f;
        m_transform.pos.z += forward.z * 2.0f;
        m_transform.SetRotation(_transform);
    }
    else
    {
        m_pEntity->transform.pos = _transform.pos;
        m_pEntity->transform.pos.x += forward.x * 2.0f;
        m_pEntity->transform.pos.z += forward.z * 2.0f;
        m_pEntity->transform.SetRotation(_transform);
    }
    
    m_pOwner = this;
    m_center = _transform.pos;
    m_radius = 0.4f;
}

void Missile::Update()
{
    float dt = cpuTime.delta;
    
    if (m_isServer)
    {
        m_center = m_transform.pos;
    }
    else
    {
        XMFLOAT3 pos = m_pEntity->transform.pos;
        m_pEmitter->pos = pos;
        m_pEmitter->dir = m_pEntity->transform.dir;
        m_pEmitter->dir.x = -m_pEmitter->dir.x; 
        m_pEmitter->dir.y = -m_pEmitter->dir.y; 
        m_pEmitter->dir.z = -m_pEmitter->dir.z;pos = m_transform.pos;

        m_center = m_pEntity->transform.pos;
    }
    
    
    float dist = sqrt(pow(m_center.x - m_firstPos.x, 2) + pow(m_center.z - m_firstPos.z, 2));
    if (dist > 25.f)
        Dead();
    
    Move(1);
}

void Missile::OnCollisionEnter(Collider* pOther)
{
    if (pOther->GetOwner() == m_pOwnerPlayer->GetOwner())
        return;
    
    if (pOther->GetOwner()->GetTag() == "Player")
    {
        if (Player* pPlayer = dynamic_cast<Player*>(pOther))
        {
            pPlayer->GetLife().TakeDamage(50); 
            if (pPlayer->IsDead())
                m_pOwnerPlayer->GetPlayerScore().AddScore(10);
        }
    }
        
    Dead();
}

void Missile::Dead(float _dt)
{
    if (m_isServer == false)
    {
        m_pEntity->dead = true;
        m_pEmitter->dead = true;
    }
    else
    {
        GameManager::Get().GetToDestroyMissilesID().push_back(m_id);
    }
    m_needToBeDestroyed = true;
}

void Missile::SetID(uint8_t _id)
{
    m_id = _id;
}

uint8_t Missile::GetID()
{
    return m_id;
}

int Missile::GetShooterId()
{ 
    return m_pOwnerPlayer->GetID();
}