#pragma once
#include "pch.h"
#include "Weapon.h"
#include "GameCamera.hpp"
#include "Life.hpp"
#include "Core/GameObject.h"
#include "Core/Score.hpp"
#include "Core/SphereCollider.h"

class GameManager;
class Weapon;

class Player : public GameObject, public SphereCollider
{
public:
    Player(bool _isServer);
    ~Player() override;
    void Init(XMFLOAT3 _pos, uint8_t id, float _angle, std::string const& _pseudo, XMINT3 _color = XMINT3(255, 255, 255));
    
    void Update() override;
    void Dead(float _dt) override;
    
    void Shoot() { m_pWeapon->Shoot(); };
    void AttachCamera();
    void Revive(XMFLOAT3 _pos);

    void SetPseudo(std::string const& _pseudo) { m_pseudo = _pseudo; };
    void SetPosition(float posX, float posY, float posZ);
    void SetDirection(float dirX, float dirY, float dirZ);
    void SetRotation(float yaw, float pitch, float roll);
    void SetColor(int r, int g, int b);

    bool IsDead() { return m_life.IsDead(); }
    Life& GetLife() { return m_life; }
    std::string& GetPseudo() { return m_pseudo; }
    int GetID() const { return m_id; }
    Score& GetPlayerScore() { return m_score; }
    int GetScore() const { return m_score.GetScore(); }

protected:

private:
    cpu_transform m_transformCanon;
    std::string m_pseudo;
    uint8_t m_id = 0;
    Score m_score;
    Life m_life;
    
    Weapon* m_pWeapon       = nullptr;
    GameCamera* m_pCamera   = nullptr;
    cpu_entity* m_pCanon    = nullptr;
    cpu_entity* m_pHead     = nullptr;
    
    float m_deadDuration    = 0.0f;
    float m_deadTime        = 3.0f;
};

