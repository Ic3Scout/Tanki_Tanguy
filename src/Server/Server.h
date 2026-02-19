#ifndef SERVER_H
#define SERVER_H

#include "Network.hpp"
#include <iostream>

constexpr size_t MAX_UDP_PACKET_SIZE = 1500;
constexpr float TIMEOUT = 5.f;

struct ClientInfo
{
	uint8_t playerId = 0;
	std::string playerName = "default";
	uint8_t playerRed;
	uint8_t playerGreen;
	uint8_t playerBlue;
	SocketAddress address;
	bool isConnected = false;
	float lastHeardTime = 0.f;
};

class GameManager;

class Server
{
public:
	Server();
	~Server();

	static Server* GetInstance() { return s_pInstance; }

	void MainLoop();

	void Start();
	void Update();
	void Exit();

	void InitializeSocket();

	static DWORD WINAPI Listen(LPVOID lpParam);
	void SendPacket(uint8_t clientID, const char* data, int length);
	void CheckClientsConnected();

	void UnpackHeader(BufferReader& reader, PacketHeader& header);
	void HandleOnPacketType(BufferReader& reader, PacketHeader& header);

	//Logique de traitement des messages
	void HandleClientHello(BufferReader& reader);
	void HandleClientInput(BufferReader& reader, PacketHeader& header);
	void HandlePing(BufferReader& reader, PacketHeader& header);
	void HandlePong(BufferReader& reader, PacketHeader& header);
	void HandleDisconnect(BufferReader& reader, PacketHeader& header);

	void SendMatchStart();
	void SendNewMissile();
	void SendMissilesToDestroy();
	void SendNewPlayer();
	void SendPlayersInfo();
	void SendServerSnapshotsToClients();
	void SendWelcomeMsg(uint8_t _clientID);

	void AddNewClient(MsgClientHello& msg);

	void SetPlayerInput(int playerIndex, uint16_t buttons);
	void SetClientInfo(ClientInfo* pInfo, const SocketAddress& addr, const MsgClientHello& msg);

	sockaddr_in m_lastClientAddr;
	SOCKET m_socket;

private:
	inline static Server* s_pInstance = nullptr;

	HANDLE m_listenThread = nullptr;
	
	GameManager* m_pGM = nullptr;
	bool m_running = false;

	uint8_t m_snapshotCount = 0;
	uint8_t m_playerCount = 0;

	std::vector<ClientInfo*> m_vClientsInfo;
};

#endif // !SERVER_SOCKET_H
