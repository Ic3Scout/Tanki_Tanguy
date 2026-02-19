#ifndef CLIENT_SOCKET_H
#define CLIENT_SOCKET_H

#include <DirectXMath.h>
#include <iostream>

#include "Network.hpp"

constexpr size_t MAX_UDP_PACKET_SIZE = 1500;

struct ServerInfo
{
	uint8_t  playerId;
	uint8_t  serverTickRate;
	uint32_t serverTimeMs;
};

class GameManager;

class ClientSocket
{
public:
	ClientSocket();
	~ClientSocket();

	static ClientSocket& GetInstance() { return *s_pClient; }

	bool ConnectToServer(const char* serverIp, int serverPort, const char* _pseudo, DirectX::XMINT3 const& _color);

	static DWORD WINAPI Listen(LPVOID lpParam);

	void RecvPacket(std::vector<uint8_t> buffer, int length);
	void SendPacket(const char* data, int length);

	void UnpackHeader(BufferReader& reader, PacketHeader& header);
	void HandleOnPacketType(BufferReader& reader, PacketHeader& header);

	//Logique de traitement des messages
	void HandleServerWelcome(BufferReader& reader);
	void HandleMatchStart(BufferReader& reader);
	void HandleServerSnapshot(BufferReader& reader);
	void HandleNewMissile(BufferReader& reader);
	void HandleDestroyMissile(BufferReader& reader);
	void HandlePlayerJoin(BufferReader& reader);
	void HandlePlayersInfo(BufferReader& reader);
	void HandlePing(BufferReader& reader);
	void HandlePong(BufferReader& reader);
	void HandleDisconnect(BufferReader& reader);

	void SendPing();

	SOCKET GetSocket() const { return m_socket; }
	ServerInfo& GetServerInfo() { return m_serverInfo; }

	bool m_isConnected = false;
	
private:
	inline static ClientSocket* s_pClient = nullptr;

	bool m_isRunning = false;
	GameManager* m_gm;

	SOCKET m_socket;
	ServerInfo m_serverInfo;
	sockaddr_in m_serverAddr;

	uint8_t m_lastSnapshotCount = 0;

	HANDLE m_listenThread = nullptr;
};

#endif // !CLIENT_SOCKET_H



