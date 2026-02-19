#pragma once
#include "pch.h"
#include "UI.hpp"
#include <functional>

class UIButton : public UI
{
public:
    UIButton();
    ~UIButton() override;
    void Init(XMINT2 _pos, XMINT2 _size, XMINT3 _color);
    void Init(XMINT2 _pos, XMINT2 _size, std::string const& _path, std::string const& _hover);
    void Update() override ;
    void Render() override;
    void SetOnClickCallback(std::function<void()> _callback);
protected:
private:
    cpu_sprite* m_pSprite   = nullptr;
    bool m_useSprite        = false;
    
    cpu_texture m_texture;
    cpu_texture m_textureHover;
    
    XMFLOAT3 m_color;

    std::function<void()> m_onClickCallback;
};
