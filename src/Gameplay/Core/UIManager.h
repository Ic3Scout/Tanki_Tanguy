#pragma once
#include "pch.h"
#include "UI.hpp"

class UIManager
{
public:
    UIManager();
    ~UIManager();
    static UIManager* Get();
    void Update();
    void Render();
    template<typename UIClass> UIClass* Create();
protected:
    
private:
    static UIManager* s_pInstance;
    
    std::vector<UI*> m_UIs;
};

#include "UIManager.inl"
