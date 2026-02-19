#pragma once
#include "GameObject.h"

template <class ObjectClass>
std::vector<GameObject*> GameManager::GetAllObjects()
{
    std::vector<GameObject*> vect;
    for (GameObject* object : m_gameObjects)
    {
        if (ObjectClass* casted = dynamic_cast<ObjectClass*>(object))
            vect.push_back(object);
    }
    return vect;
}

template <class ComponentClass>
std::vector<ComponentClass*> GameManager::GetAllComponents()
{
    std::vector<ComponentClass*> vect;
    for (GameObject* object : m_gameObjects)
    {
        if (ComponentClass* casted = dynamic_cast<ComponentClass*>(object))
            vect.push_back(casted);
    }
    return vect;
}

template <class ObjectClass>
ObjectClass* GameManager::CreateObject(bool _isServer)
{
    ObjectClass* newObject = new ObjectClass(_isServer);

    GameObject* casted = dynamic_cast<GameObject*>(newObject);
    if (casted == nullptr)
    {
        delete newObject;
        return nullptr;
    }
    m_gameObjects.push_back(casted);
    return newObject;

}