#pragma once
#include "pch.h"

class Life
{
public:
    Life(int _nbrLife);
    ~Life() = default;

    void Heal(int _heal);
    void TakeDamage(int _damage);
    void ResetLIfe() { m_currentLife = m_maxLife; }
    
    int GetMaxLife() const { return m_maxLife; };
    int GetCurrentLife() const { return m_currentLife; };
    bool IsDead() { return m_currentLife <= 0; }
    
protected:
    
private:
    int m_maxLife;
    int m_currentLife;
};

inline Life::Life(int _nbrLife)
{
    m_maxLife = _nbrLife;
    m_currentLife = m_maxLife;
}

inline void Life::Heal(int _heal)
{
    if (m_maxLife > m_currentLife + _heal)
        m_currentLife += _heal;
    else
        m_currentLife = m_maxLife;
}

inline void Life::TakeDamage(int _damage)
{
    if (0 < m_currentLife - _damage)
        m_currentLife -= _damage;
    else
        m_currentLife = 0;
}
