#pragma once
#include "pch.h"
#include "UI.hpp"

class UIText : public UI
{
    public:
    UIText() { };
    ~UIText()       override { };
    void Update()   override { };
    void Render()   override;

    void Init(XMINT2 _pos, int _anchor, std::string const& _text);
    void Init(XMINT2 _pos, int _anchor, int _value);

    void Create(int _size, XMFLOAT3 _color);
    void SetFont(cpu_font& _font);
    
    void SetText(std::string const& _text);
    void SetText(int _value);

private:
    std::string m_text;
    cpu_font m_font;
    
    int m_intText       = -1;
    int m_anchor        = 0;
};

inline void UIText::Render()
{
    cpuDevice.DrawText(&m_font, m_text.c_str(), m_pos.x, m_pos.y, m_anchor);
}

inline void UIText::Init(XMINT2 _pos, int _anchor, std::string const& _text)
{
    m_pos = _pos;
    m_anchor = _anchor;
    m_text = _text;
}

inline void UIText::Init(XMINT2 _pos, int _anchor, int _value)
{
    m_pos = _pos;
    m_anchor = _anchor;
    m_text = std::to_string(_value);
}

inline void UIText::Create(int _size, XMFLOAT3 _color)
{
    m_font.Create(_size, _color);
}

inline void UIText::SetFont(cpu_font& _font)
{
    m_font = _font;
}

inline void UIText::SetText(std::string const& _text)
{
    m_text = _text;
}

inline void UIText::SetText(int _value)
{
    m_text = std::to_string(_value);
}
