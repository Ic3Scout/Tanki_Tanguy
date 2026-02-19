#pragma once
#include "pch.h"
#include "Core/UI.hpp"

class InputText : public UI
{
public:
    InputText() = default;
    ~InputText() override = default ;
    
    void Init(cpu_font& _font, XMINT2 _pos, int _anchor, std::string const& _text);
    void Update() override;
    void Render() override;

    void Reset();
    bool IsFinished() const { return m_finished;};
    const std::string& GetText() const { return m_text; }

private:
    std::string m_question;
    std::string m_text;
    cpu_font* m_font;
    int m_anchor    = 0;
    bool m_finished = false;
};


