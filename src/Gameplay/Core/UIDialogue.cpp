#include "UIDialogue.h"

#include "Client/App.h"

UIDialogue::UIDialogue()
{
    m_isActive = true;
}

UIDialogue::~UIDialogue()
{
    for (InputText* inputTxt : m_dialogue)
        CPU_DELPTR(inputTxt);
    m_dialogue.clear();
    m_answers.clear();
}

void UIDialogue::AddInputText(cpu_font& _font, XMINT2 _pos, int _anchor, std::string _text, std::string& _answer)
{
    InputText* newInputText = new InputText();
    newInputText->Init(_font, _pos, _anchor, _text);
    m_dialogue.push_back(newInputText);
    m_answers.push_back(&_answer);
}

void UIDialogue::Update()
{
    if (m_dialogue[m_currentIndex]->IsFinished())
    {
         *m_answers[m_currentIndex] = m_dialogue[m_currentIndex]->GetText();
         if (m_dialogue.size() > m_currentIndex + 1)
            m_currentIndex++;
         else
            m_isActive = false;
    }
    m_dialogue[m_currentIndex]->Update();
}

void UIDialogue::Render()
{
    m_dialogue[m_currentIndex]->Render();
}

void UIDialogue::Reset()
{
    for (InputText* dialog : m_dialogue)
        dialog->Reset();

    m_currentIndex = 0;
    m_isActive = true;
}
