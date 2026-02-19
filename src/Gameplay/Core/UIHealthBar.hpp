#pragma once
#include "pch.h"
#include "Player.h"
#include "UI.hpp"

class UIHealthBar : public UI
{
public:
    ~UIHealthBar() override;
    void Init(cpu_font& _font, XMINT2 _pos, float _size, Player& _player, std::string const& _pathGreen, std::string const& _pathGray);
    void Update() override;
    void Render() override;

private:
    std::vector<cpu_sprite*> m_sprites;
    cpu_texture m_textGreen;
    cpu_texture m_textGray;
    cpu_font m_font;
    Player* m_pPlayer = nullptr;
};


inline UIHealthBar::~UIHealthBar()
{
    for (cpu_sprite* pSprite : m_sprites)
        cpuEngine.Release(pSprite);

    m_sprites.clear();
}

inline void UIHealthBar::Init(cpu_font& _font, XMINT2 _pos, float _size, Player& _player, std::string const& _pathGreen, std::string const& _pathGray)
{
    m_pos = _pos;
    m_font = _font;
    
    m_textGreen = cpu_texture();
    m_textGreen.Load(_pathGreen.c_str());
    m_textGray = cpu_texture();
    m_textGray.Load(_pathGray.c_str());
    
    m_pPlayer = &_player;
    for (int i = 0; i < 5; i++)
    {
        cpu_sprite* sprite = cpuEngine.CreateSprite();
        sprite->pTexture = &m_textGreen;
        sprite->x = static_cast<int>(_pos.x) + i * static_cast<int>(_size);
        sprite->y = static_cast<int>(_pos.y);
        if (m_pPlayer == nullptr)
            sprite->visible = false;
        
        m_sprites.push_back(sprite);
    }
}

inline void UIHealthBar::Update()
{
    if (m_pPlayer == nullptr)
        return;
    
    int currentHP = m_pPlayer->GetLife().GetCurrentLife();
    int maxHP = m_pPlayer->GetLife().GetMaxLife();

    int hpPerSprite = std::max(1, maxHP / static_cast<int>(m_sprites.size()));

    for (int i = 0; i < m_sprites.size(); i++)
    {
        m_sprites[i]->visible = true;
        if (currentHP > i * hpPerSprite)
            m_sprites[i]->pTexture = &m_textGreen;
        else
            m_sprites[i]->pTexture = &m_textGray;
    }
}

inline void UIHealthBar::Render()
{
    if (m_pPlayer == nullptr)
        return;
    
    std::string text = std::to_string(m_pPlayer->GetLife().GetCurrentLife());
    cpuDevice.DrawText(&m_font, text.c_str(), m_pos.x, m_pos.y + 6, CPU_TEXT_LEFT);
}
