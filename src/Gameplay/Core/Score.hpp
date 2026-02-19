#pragma once

class Score
{
public:
    Score() = default;
    ~Score() { }
    
    void AddScore(int _score)   { m_score += _score; }
    void SetScore(int _score)   { m_score = _score; }
    void ResetScore()           { m_score = 0; }
    int GetScore() const        { return m_score; }
    
private:
    int m_score = 0;
};

