#include "UIButton.h"

UIButton::UIButton()
{
    m_color         = XMFLOAT3(0, 0, 0);
}

UIButton::~UIButton()
{
    cpuEngine.Release(m_pSprite);
}

void UIButton::Init(XMINT2 _pos, XMINT2 _size, XMINT3 _color)
{
    m_pos           = _pos;
    m_size          = _size;
    m_color         = cpu::ToColor(_color.x, _color.y, _color.z);
    m_useSprite = false;
}

void UIButton::Init(XMINT2 _pos, XMINT2 _size, std::string const& _path, std::string const& _hover)
{
    m_pos               = _pos;
    m_size              = _size;
    m_texture.Load(_path.c_str());
    m_textureHover.Load(_hover.c_str());
    m_pSprite           = cpuEngine.CreateSprite();
    m_pSprite->pTexture = &m_texture;
    m_pSprite->x        = _pos.x;
    m_pSprite->y        = _pos.y;
    m_useSprite         = true;
}

void UIButton::Update()
{
    XMFLOAT2 ray;
    cpuEngine.GetCursor(ray);
    XMINT2 posCursor = XMINT2(static_cast<int>(ray.x), static_cast<int>(ray.y));

    if (posCursor.x >= m_pos.x && posCursor.x <= m_pos.x + m_size.x &&
        posCursor.y >= m_pos.y && posCursor.y <= m_pos.y + m_size.y)
    {
        if (m_useSprite)
            m_pSprite->pTexture = &m_textureHover;
        
        if (cpuInput.IsKeyDown(VK_LBUTTON) && m_onClickCallback)
            m_onClickCallback();
    }
    else
        if (m_useSprite)
            m_pSprite->pTexture = &m_texture;
}

void UIButton::Render()
{
    if (!m_useSprite)
        cpuDevice.DrawRectangle(m_pos.x, m_pos.y, m_size.x, m_size.y, m_color);
}

void UIButton::SetOnClickCallback(std::function<void()> _callback)
{
    m_onClickCallback = _callback;
}


