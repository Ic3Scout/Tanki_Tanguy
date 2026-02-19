#include "pch.h"
#include "PlayerController.h"

#include "Player.h"
#include "Network/ClientSocket.h"
#include "Core/GameManager.h"

PlayerController::~PlayerController()
{
    m_pendingInputs.clear();
    m_incomingInputs.clear();
}

void PlayerController::HandleInput()
{
    if (cpuInput.IsKey(VK_UP))
    {
        //m_pPlayer->Move(1);
        m_pendingInputs.push_back(static_cast<InputCmd>(VK_UP));
    }
    if (cpuInput.IsKey(VK_DOWN))
    {
        //m_pPlayer->Move(-1);
        m_pendingInputs.push_back(static_cast<InputCmd>(VK_DOWN));
    }
    if (cpuInput.IsKey(VK_RIGHT))
    {
        //m_pPlayer->Turn(1);
        m_pendingInputs.push_back(static_cast<InputCmd>(VK_RIGHT));
    }
    if (cpuInput.IsKey(VK_LEFT))
    {
        //m_pPlayer->Turn(-1);
        m_pendingInputs.push_back(static_cast<InputCmd>(VK_LEFT));
    }
    if (cpuInput.IsKey(VK_SPACE))
    {
        //m_pPlayer->Shoot();
        m_pendingInputs.push_back(static_cast<InputCmd>(VK_SPACE));
    }

    if (cpuInput.IsKeyDown('N'))
    {
        m_pPlayer->GetLife().TakeDamage(10);
    }
}

void PlayerController::ApplyInput()
{
    for (const InputCmd& inputCmd : m_incomingInputs)
    {
        int input = static_cast<int>(inputCmd.buttons);

        if (input == VK_UP) // UP
            m_pPlayer->Move(1);
        if (input == VK_DOWN) // DOWN
            m_pPlayer->Move(-1);
        if (input == VK_RIGHT) // RIGHT
            m_pPlayer->Turn(1);
        if (input == VK_LEFT) // LEFT
            m_pPlayer->Turn(-1);

        if (input == VK_SPACE) // SHOOT
            m_pPlayer->Shoot();
    }

    m_incomingInputs.clear();
}

void PlayerController::SendInputList()
{
    if (ShouldSend() && m_pendingInputs.empty() == false)
    {
        ClientSocket* clientSocket = &ClientSocket::GetInstance();

        PacketHeader header;
        header.packetType = PacketType::ClientInput;
        header.playerId = clientSocket->GetServerInfo().playerId;

        MsgInputList msg;
        msg.inputsList = m_pendingInputs;

        BufferWriter writer;
        SerializePacketHeader(writer, header);
        SerializeInputList(writer, msg);
        clientSocket->SendPacket((const char*)writer.data.data(), (int)writer.data.size());

        m_pendingInputs.clear();
    }
}

void PlayerController::AddIncomingInputs(uint16_t input)
{
    InputCmd cmd;
    cmd.buttons = input;
    m_incomingInputs.push_back(cmd);
}

bool PlayerController::ShouldSend() const
{
    static DWORD lastSendTime = 0;
    DWORD currentTime = GetTickCount64();
    if (currentTime - lastSendTime >= 1) // envoi toutes les 1 ms
    {
        lastSendTime = currentTime;
        return true;
    }
    return false;
}
