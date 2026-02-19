#ifndef CHRONO_CPP_DEFINED
#define CHRONO_CPP_DEFINED

#include "Chrono.h"

#include <Windows.h>
#include <mmsystem.h>

Chrono::Chrono() : m_lastTime(timeGetTime()), m_pauseTime(0)
{
}

Chrono::~Chrono()
{
}

void Chrono::Start()
{
    if (!m_isPaused) return;

    m_lastTime += timeGetTime() - m_pauseTime;
    m_isPaused = false;
}

void Chrono::Pause()
{
    if (m_isPaused) return;

    m_pauseTime = timeGetTime();
    m_isPaused = true;
}

float Chrono::GetElapsedTime()
{
    if (m_isPaused)
    {
        unsigned __int64 elapsedtime = m_pauseTime - m_lastTime;
        return elapsedtime / 1000.0f;
    }

    unsigned __int64 currentTime = timeGetTime();
    unsigned __int64 elapsedTime = currentTime - m_lastTime;
    return elapsedTime / 1000.0f;
}

float Chrono::Reset()
{
    unsigned __int64 currentTime = timeGetTime();

    if (m_isPaused)
    {
        unsigned __int64 elapsedTime = m_pauseTime - m_lastTime;
        m_lastTime = currentTime;
        m_pauseTime = currentTime;
        return elapsedTime / 1000.0f;
    }

    unsigned __int64 elapsedTime = currentTime - m_lastTime;
    m_lastTime = currentTime;
    return elapsedTime / 1000.0f;
}

#endif
