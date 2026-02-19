#include "GameManager.h"

#include <random>
#include "Collider.h"
#include "Utils.hpp"
#include "GameObject.h"
#include "Player.h"
#include "Map.h"
#include "PlayerController.h"
#include "Missile.h"

GameManager* GameManager::s_pInstance = nullptr;

GameManager::GameManager(bool _isServer) : m_chrono()
{
    m_isServer = _isServer;
    s_pInstance = this;
}

GameManager::~GameManager()
{
    for (GameObject* go : m_gameObjects)
        CPU_DELPTR(go);
    m_gameObjects.clear();
    m_missiles.clear();
    m_players.clear();

    CPU_DELPTR(m_pPlCrt);
    for (PlayerController* plCrt : m_playerControllers)
        CPU_DELPTR(plCrt);
    m_playerControllers.clear();

    CPU_DELPTR(m_pMap);
    s_pInstance = nullptr; 
}

GameManager& GameManager::Get()
{
    return *s_pInstance;
}

void GameManager::Start()
{
    delete m_pPlCrt;
    m_pPlCrt = new PlayerController();

    m_pMap = new Map();
    m_pMap->Init(50, 50);
}

void GameManager::Update()
{
    float dt = cpuTime.delta;
    CollisionCheck();
    m_pPlCrt->HandleInput();
    m_pPlCrt->SendInputList();

    CheckMissileToDestroy();

    for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); )
    {
        GameObject* pGameObject = *it;
        if (pGameObject->IsNeedToBeDestroyed())
        {
            it = m_gameObjects.erase(it);
            delete pGameObject;
        }
            
        else
        {
            pGameObject->Update();
            ++it;
        }
    }
}

void GameManager::ServerSideStart()
{
    delete m_pPlCrt;
    m_chrono.Start();

    m_pMap = new Map();
    m_pMap->Init(50, 50);
}

void GameManager::ServerSideUpdate()
{
    dt = m_chrono.Reset();
    CollisionCheck();

    for (PlayerController* plCrt : m_playerControllers)
    {
        plCrt->ApplyInput();
    }
    
    for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); )
    {
        GameObject* pGameObject = *it;
        if (pGameObject->IsNeedToBeDestroyed())
        {
            it = m_gameObjects.erase(it);
            delete pGameObject;
        }
            
        else
        {
            pGameObject->Update();
            ++it;
        }
    }
}

bool GameManager::WorldToScreen( XMFLOAT3 const& world, XMFLOAT2& screen, XMFLOAT4X4 const& viewProj, int screenWidth, int screenHeight)
{
    float clip[4];

    clip[0] = world.x * viewProj.m[0][0] + world.y * viewProj.m[1][0] + world.z * viewProj.m[2][0] + viewProj.m[3][0];
    clip[1] = world.x * viewProj.m[0][1] + world.y * viewProj.m[1][1] + world.z * viewProj.m[2][1] + viewProj.m[3][1];
    clip[2] = world.x * viewProj.m[0][2] + world.y * viewProj.m[1][2] + world.z * viewProj.m[2][2] + viewProj.m[3][2];
    clip[3] = world.x * viewProj.m[0][3] + world.y * viewProj.m[1][3] + world.z * viewProj.m[2][3] + viewProj.m[3][3];

    if (clip[3] <= 0.0f)
        return false;

    float ndcX = clip[0] / clip[3];
    float ndcY = clip[1] / clip[3];

    screen.x = (ndcX * 0.5f + 0.5f) * static_cast<float>(screenWidth);
    screen.y = (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(screenHeight);

    return true;
}

void GameManager::CollisionCheck()
{
    std::vector<Collider*> colliders = GetAllComponents<Collider>();

    for (Collider* pCollider : colliders)
    {
        if (pCollider->m_isActiveCollider == false)   continue;
        
        for (Collider* pOther : colliders)
        {
            if (pCollider == pOther) continue;
            
            if (pCollider->IsColliding(pOther))
            {
                if (pCollider->IsAlreadyColliding(pOther) == false)
                    pCollider->OnCollisionEnter(pOther);
                if (pCollider->IsAlreadyColliding(pOther) == true)
                    pCollider->OnCollisionStay(pOther);
            }
            else
            {  
                if (pCollider->IsAlreadyColliding(pOther) == true)
                    pCollider->OnCollisionExit(pOther);
            }
        }
    }
}

Player* GameManager::AddPlayer(XMFLOAT3 _pos, float _angle, XMINT3 _color, std::string _pseudo, bool _isServer, bool _mainPlayer)
{
    Player* player = CreateObject<Player>(_isServer);
    player->Init(_pos, m_players.size(), _angle, _pseudo, _color);
    if (m_isServer)
    {
        PlayerController* plCrt = new PlayerController();
        plCrt->Init(*player);
        m_playerControllers.push_back(plCrt);
    }
    m_players.push_back(player);
    return player;
}

void GameManager::SetMainPlayer(int _id)
{
    if (_id >= m_players.size())
        return;

    m_players[_id]->AttachCamera();
    m_mainPlayer = m_players[_id];
    m_pPlCrt->Init(*m_mainPlayer);
}

Missile* GameManager::AddMissile()
{
    Missile* missile = CreateObject<Missile>(m_isServer);

    missile->SetID(m_nextMissileID);
    m_missiles.push_back(missile);
    m_nextMissileID++;

    if (m_isServer)
    {
        m_newMissileAdded = true;
        m_lastMissileAdded = missile;
    }

    return missile;
}

void GameManager::CheckMissileToDestroy()
{
    for (auto it = m_missiles.begin(); it != m_missiles.end(); )
    {
        Missile* m = *it;
        bool mustDestroy = m->IsNeedToBeDestroyed();

        if (mustDestroy == false)
        {
            for (uint16_t id : m_toDestroyMissilesID)
            {
                if (m->GetID() == id)
                {
                    m->Dead();
                    mustDestroy = true;
                    break;
                }
            }
        }
        if (mustDestroy)
            it = m_missiles.erase(it);
        else
            ++it;
    }
    m_toDestroyMissilesID.clear();
}

void GameManager::RespawnPlayer(Player* _player, float _minDistanceBetweenPlayers)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, m_pMap->m_map.size() - 1);
    constexpr int MAX_ATTEMPTS  = 100;
    int attempts                = 0;

    while (attempts < MAX_ATTEMPTS)
    {
        attempts++;
        
        XMINT2 tile         = m_pMap->m_map[dist(gen)];
        XMFLOAT2 spawnPos   = { (float)tile.x, (float)tile.y };

        bool valid = true;
        for (Player* other : m_players)
        {
            if (other == _player)
                continue;
            
            if (other->IsDead())
                continue;

            XMFLOAT2 otherPos = XMFLOAT2(other->GetPos().x, other->GetPos().z);
            float d = Utils::Distance(spawnPos, otherPos);
            if (d < _minDistanceBetweenPlayers)
            {
                valid = false;
                break;
            }
        }

        if (valid)
        {
            _player->Revive(XMFLOAT3(spawnPos.x, 0.0f,  spawnPos.y));
            return;
        }
    }
    _player->Revive({ 0.f, 0.f, 0.f });
}

