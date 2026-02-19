#pragma once
#include "pch.h"
#include "Chrono.h"

class GameObject;
class Collider;
class Player;
class PlayerController;
class Missile;
struct Map;

class GameManager
{
public:
    GameManager(bool _isServer);
    ~GameManager();

    static GameManager& Get();
    
    void Start();
    void Update();

	void ServerSideStart();
	void ServerSideUpdate();
    
    bool WorldToScreen(XMFLOAT3 const& world, XMFLOAT2& screen, XMFLOAT4X4 const& viewProj, int screenWidth, int screenHeight);
    
    void CollisionCheck();
    Player* AddPlayer(XMFLOAT3 _pos, float _angle, XMINT3 _color, std::string _pseudo, bool _isServer, bool _mainPlayer = false);
    void SetMainPlayer(int _id);
    Missile* AddMissile();
    void CheckMissileToDestroy();
    void RespawnPlayer(Player* _player, float _minDistanceBetweenPlayers = 30.0f);

    Player* GetMainPlayer() const { return m_mainPlayer; }
    std::vector<Player*> GetPlayers() { return m_players; }
    std::vector<PlayerController*> GetPlayerControllers() { return m_playerControllers; }
    bool GetNewMissileAdded() { return m_newMissileAdded; }
    Missile* GetLastMissileAdded() { return m_lastMissileAdded; }
    std::vector<Missile*> GetMissiles() { return m_missiles; }
    std::vector<uint16_t>& GetToDestroyMissilesID() { return m_toDestroyMissilesID; }

    void SetToDestroyMissilesID(std::vector<uint16_t> v) { m_toDestroyMissilesID = v; }
    void SetNewMissileAdded(bool v) { m_newMissileAdded = v; }
    void SetMatchStart(bool v) { m_matchStarted = v; }

    float GetDeltaTime() { return dt; };

    template <class ObjectClass> ObjectClass* CreateObject(bool _isServer);
    template <class ObjectClass> std::vector<GameObject*> GetAllObjects();
    template <class ComponentClass> std::vector<ComponentClass*> GetAllComponents();
protected:
    
private:
    static GameManager* s_pInstance;
    float dt = 0.f;

    bool m_isServer = false;
    bool m_matchStarted = false;
    
    std::vector<GameObject*> m_gameObjects;
    Map* m_pMap = nullptr;
    
    PlayerController* m_pPlCrt = nullptr;
    Player* m_mainPlayer = nullptr;
    std::vector<PlayerController*> m_playerControllers;
    std::vector<Player*> m_players;
    std::vector<Missile*> m_missiles;
    std::vector<uint16_t> m_toDestroyMissilesID;

    bool m_newMissileAdded = false;
    uint8_t m_nextMissileID = 0;
    Missile* m_lastMissileAdded = nullptr;

    Chrono m_chrono;
};

#include "GameManager.inl"
