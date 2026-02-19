#pragma once 
#include "pch.h"

#include "Missile.h"

class Player;
class GameManager;

class Weapon
{
public:
    Weapon(cpu_transform& _wpTransform, Player& _owner, bool _isServer) : m_pWeaponTransform(&_wpTransform), m_pOwner(&_owner), m_isServer(_isServer) {};
    ~Weapon();
    void Update();
    void Shoot();
protected:
    
private:
    cpu_transform* m_pWeaponTransform;
    Player* m_pOwner;
    bool m_isServer;
    float m_fireDuration = 1.5f;
    float m_fireDurationMax = 1.5f;
};

