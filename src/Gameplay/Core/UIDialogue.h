#pragma once
#include "InputText.h"

class App;

class UIDialogue : public UI
{
public:
    UIDialogue();
    ~UIDialogue() override;
    void AddInputText(cpu_font& _font, XMINT2 _pos, int _anchor, std::string _text, std::string& _answer);
    void Update() override;
    void Render() override;

    void Reset();

protected:

private:
    int m_currentIndex = 0;
    std::vector<InputText*> m_dialogue;
    std::vector<std::string*> m_answers;
};