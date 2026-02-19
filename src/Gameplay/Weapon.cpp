#include "Weapon.h"
#include "Core/GameManager.h"

Weapon::~Weapon() {} 

void Weapon::Shoot()
{
    if (m_fireDuration >= m_fireDurationMax)
    {
        GameManager& s_pI = GameManager::Get();
        Missile* missile = s_pI.AddMissile();
        missile->Init(*m_pWeaponTransform, *m_pOwner);

        m_fireDuration = 0.0f;
    }
}

void Weapon::Update()
{
    float dt = 0.f;
    if (m_isServer)
        dt = GameManager::Get().GetDeltaTime();
    else
        dt = cpuTime.delta;
    m_fireDuration += dt;
}