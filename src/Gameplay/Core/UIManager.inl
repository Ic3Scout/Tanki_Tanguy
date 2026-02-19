#pragma once
#include "UI.hpp"

template <typename UIClass>
UIClass* UIManager::Create()
{
    UIClass* ui = new UIClass();
    UI* casted = dynamic_cast<UI*>(ui);
    if (casted == nullptr)
    {
        delete ui;
        return nullptr;
    }
    m_UIs.push_back(casted);
    return ui;
}