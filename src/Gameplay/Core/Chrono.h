#ifndef CHRONO_H_DEFINED
#define CHRONO_H_DEFINED

class Chrono
{
public:
    Chrono();
    ~Chrono();

    void Start();
    void Pause();

    float GetElapsedTime();
    float Reset();

private:
    bool m_isPaused = false;
    unsigned __int64 m_lastTime;
    unsigned __int64 m_pauseTime;
};

#endif

