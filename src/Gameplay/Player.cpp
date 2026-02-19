#include "Player.h"
#include "Weapon.h"
#include "Core/GameManager.h"

Player::Player(bool _isServer) : GameObject(_isServer), SphereCollider(XMFLOAT3(), 1.0f), m_score(Score()), m_life(Life(100)) { }

Player::~Player()
{
    CPU_DELPTR(m_pWeapon);
    CPU_DELPTR(m_pCamera);
    if (m_isServer)
    {
        cpuEngine.Release(m_pCanon);
        cpuEngine.Release(m_pHead);
    }
}

void Player::Init(XMFLOAT3 _pos, uint8_t id, float _angle, std::string const& _pseudo, XMINT3 _color)
{
    m_tag = "Player";
    m_pseudo = _pseudo;
    m_id = id;

    m_angle = _angle;
    m_speed = 2.5f;
    m_transform.pos = _pos;
    m_center = _pos;

    m_pOwner = this;
    
    if (m_isServer)
    {
        m_transformCanon.pos = _pos;
        m_transform.SetYPR(_angle, 0, 0);
        m_transformCanon.pos.y += 0.5f;
        m_pWeapon = new Weapon(m_transformCanon, *this, m_isServer);
    }
    else
    {
        m_material.color = cpu::ToColor(_color.x, _color.y, _color.z);
        m_mesh.CreateCube();

        m_pEntity->pMesh = &m_mesh;
        m_pEntity->pMaterial = &m_material;
        m_pEntity->transform.pos = _pos;
        m_pEntity->transform.sca = XMFLOAT3(1.0f, 1.0f, 2.0f);
        m_pEntity->transform.SetYPR(_angle, 0.0f, 0.0f);

        XMFLOAT3 forward = m_pEntity->transform.dir;
        
        m_pHead = cpuEngine.CreateEntity();
        m_pHead->pMesh = &m_mesh;
        m_pHead->pMaterial = &m_material;
        m_pHead->transform.pos = _pos;
        m_pHead->transform.pos.y += 0.5f;
        m_pHead->transform.SetYPR(_angle, 0.0f, 0.0f);
        m_pHead->transform.sca = XMFLOAT3(0.75f, 0.5f, 0.75f);
    
        m_pCanon = cpuEngine.CreateEntity();
        m_pCanon->pMesh = &m_mesh;
        m_pCanon->pMaterial = &m_material;
        m_pCanon->transform.pos = _pos;
        m_pCanon->transform.SetYPR(_angle, 0.0f, 0.0f);
        m_pCanon->transform.pos.x += forward.x * 1.5f;
        m_pCanon->transform.pos.z += forward.z * 1.5f;
        m_pCanon->transform.pos.y += 0.5f;
        m_pCanon->transform.sca = XMFLOAT3(0.25f, 0.25f, 3.0f);
        
        m_pWeapon = new Weapon(m_pCanon->transform, *this, m_isServer);
    }
}

void Player::Update()
{
    float dt = 0.f;
    GameObject::Update();
    
    XMFLOAT3 forward;
    XMFLOAT3 pos;
    
    if (m_isServer)
    {
        dt = GameManager::Get().GetDeltaTime();
        forward = m_transform.dir;
        pos = m_transform.pos;

        m_transform.LookAt(
            m_transform.pos.x + forward.x,
            m_transform.pos.y,
            m_transform.pos.z + forward.z
        );
        
        m_transformCanon.pos = pos;
        m_transformCanon.pos.x += forward.x * 1.5f;
        m_transformCanon.pos.y += 0.5f;
        m_transformCanon.pos.z += forward.z * 1.5f;

        m_transformCanon.LookAt(
            m_transformCanon.pos.x + forward.x,
            m_transformCanon.pos.y,
            m_transformCanon.pos.z + forward.z
        );
        
        m_center = m_transform.pos;
    }
    else
    {
        dt = cpuTime.delta;
        forward = m_pEntity->transform.dir;
        pos = m_pEntity->transform.pos;

        m_pEntity->transform.LookAt(
            m_pEntity->transform.pos.x + forward.x,
            m_pEntity->transform.pos.y,
            m_pEntity->transform.pos.z + forward.z
        );
        
        m_pHead->transform.pos = pos; 
        m_pHead->transform.pos.y += 0.5f;
        m_pHead->transform.rot = m_pEntity->transform.rot;
        
        m_pCanon->transform.pos = pos;
        m_pCanon->transform.pos.x += forward.x * 1.5f;
        m_pCanon->transform.pos.z += forward.z * 1.5f;
        m_pCanon->transform.pos.y += 0.5f;
        
        m_pCanon->transform.LookAt(
            m_pCanon->transform.pos.x + forward.x,
            m_pCanon->transform.pos.y,
            m_pCanon->transform.pos.z + forward.z
        );
        
        m_center = m_pEntity->transform.pos;
    }
    m_pWeapon->Update();
    
    if (m_life.IsDead())
    {
        Dead(dt);
        return;
    }

    if (m_pCamera && m_isServer == false)
        m_pCamera->FollowPlayer();
}

void Player::AttachCamera()
{
    m_pCamera = new GameCamera(m_pEntity->transform);
}

void Player::Revive(XMFLOAT3 _pos)
{
    if (m_isServer)
        m_transform.pos = _pos;
    else
        m_pEntity->transform.pos = _pos;
    m_life.ResetLIfe();
}

void Player::SetPosition(float posX, float posY, float posZ)
{
    XMFLOAT3 newPos = XMFLOAT3(posX, posY, posZ);
    m_pEntity->transform.pos = newPos;
}

void Player::SetDirection(float dirX, float dirY, float dirZ)
{
    XMFLOAT3 newDir = XMFLOAT3(dirX, dirY, dirZ);
    m_pEntity->transform.dir = newDir;
}

void Player::SetRotation(float yaw, float pitch, float roll)
{
    m_pEntity->transform.SetYPR(yaw, pitch, roll);
}

void Player::SetColor(int r, int g, int b)
{
    m_color = cpu::ToColor(r, g, b);
    if (!m_isServer)
        m_material.color = m_color;
}

void Player::Dead(float _dt)
{
    if (m_deadDuration > m_deadTime)
    {
        GameManager::Get().RespawnPlayer(this);
        m_deadDuration = 0.0f;
        return;
    }
    if (m_isServer)
        m_transform.pos.y = -2.0f;
    else
        m_pEntity->transform.pos.y = -2.0f;
    
    m_deadDuration += _dt;
}
