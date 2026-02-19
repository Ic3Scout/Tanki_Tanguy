#pragma once
#include "pch.h"

class UI
{
public:
    UI() = default;
    virtual ~UI() = default;
    virtual void Update() =0;
    virtual void Render() = 0;
    
    bool IsActive() { return m_isActive; }
    void SetActive(bool _state) { m_isActive = _state; }
    void SetToBeDestroyed() { m_needToBeDestroyed = true; }
    bool IsNeedToBeDestroyed() { return m_needToBeDestroyed; }

protected:
    bool m_isActive = true;

    bool m_needToBeDestroyed = false;
    
    XMINT2 m_pos;
    XMINT2 m_size;
    
private:
};
