#pragma once
#include "pch.h"

struct Map
{
    int m_width;
    int m_height;
    std::vector<XMINT2> m_map;

    void Init(int _width, int _height)
    {
        m_width = _width;
        m_height = _height;

        XMINT2 mid = XMINT2(m_width / 2, m_height / 2);
        for (int i = -mid.x; i < m_width; i++)
        {
            for (int j = -mid.y; j < m_height; j++)
            {
                m_map.push_back(XMINT2(i, j));
            }
        }
    }
};

