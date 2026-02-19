#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H

#include <vector>
#include "Network/Network.hpp"

class Player;

class PlayerController
{
public:
    PlayerController() = default;
    ~PlayerController();

    void Init(Player& _player) { m_pPlayer = &_player; };

    void HandleInput();
    void ApplyInput();

    void SendInputList();
    void AddIncomingInputs(uint16_t input);

    bool ShouldSend() const;
protected:

private:
    Player* m_pPlayer;

    std::vector<InputCmd> m_pendingInputs;
    std::vector<InputCmd> m_incomingInputs;
};

#endif // !PLAYER_CONTROLLER_H