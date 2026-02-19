#pragma once
#include "pch.h"

class GameCamera
{
public:
    GameCamera(cpu_transform& _bodyTransform);
    ~GameCamera() = default;
    void Update();
    void FollowPlayer();

protected:
    
private:
    
    cpu_camera* m_pCamera;
    cpu_transform* m_pBodyTransform;
};


inline GameCamera::GameCamera(cpu_transform& _bodyTransform)
{
    m_pCamera = cpuEngine.GetCamera();
    m_pBodyTransform = &_bodyTransform;
}

inline void GameCamera::Update()
{
    
}

inline void GameCamera::FollowPlayer()
{
    float camDist = 1.75f;
    float camHeight = 1.25f;

    XMFLOAT3 forward = m_pBodyTransform->dir;
    
    m_pCamera->transform.pos.x = m_pBodyTransform->pos.x - forward.x * camDist;
    m_pCamera->transform.pos.z = m_pBodyTransform->pos.z - forward.z * camDist;
    m_pCamera->transform.pos.y = m_pBodyTransform->pos.y + camHeight;
    
    XMFLOAT3 lookAtPos;
    lookAtPos.x = m_pBodyTransform->pos.x + forward.x * 5.f;
    lookAtPos.y = m_pBodyTransform->pos.y;
    lookAtPos.z = m_pBodyTransform->pos.z + forward.z * 5.f;

    m_pCamera->transform.LookAt( lookAtPos.x, lookAtPos.y, lookAtPos.z );
}
