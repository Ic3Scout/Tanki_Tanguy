#include "InputText.h"

void InputText::Init(cpu_font& _font, XMINT2 _pos, int _anchor, std::string const& _question)
{
    m_pos = _pos;
    m_font = &_font;
    m_anchor = _anchor;
    m_question = _question;
    m_text.clear();
}

void InputText::Update()
{
    if (m_finished)
        return;

    // Lettres A-Z
    for (char c = 'A'; c <= 'Z'; c++)
        if (cpuInput.IsKeyDown(c))
            m_text += c;

    // Chiffres
    for (char c = '0'; c <= '9'; c++)
        if (cpuInput.IsKeyDown(c))
            m_text += c;
    
    int temp = 0;
    for (int i = VK_NUMPAD0; i <= VK_NUMPAD9; i++)
    {
        if (cpuInput.IsKeyDown(i))
            m_text += std::to_string(temp);
        temp++;
    }
    
    if (cpuInput.IsKeyDown(VK_OEM_PERIOD) || cpuInput.IsKeyDown(VK_DECIMAL))
        m_text += '.';

    // Backspace
    if (cpuInput.IsKeyDown(VK_BACK))
        if (!m_text.empty())
            m_text.pop_back();
    
    // Validation
    if (cpuInput.IsKeyDown(VK_RETURN))
        m_finished = true;
}

void InputText::Render()
{
    const std::string& textToDisplay =
        m_text.empty() ? m_question : m_text;

    cpuDevice.DrawText(
        m_font,
        textToDisplay.c_str(),
        static_cast<int>(m_pos.x),
        static_cast<int>(m_pos.y),
        m_anchor
    );
}

void InputText::Reset()
{
    m_text.clear();
    m_finished = false;
}

