#include "ClientSocket.h"

#include "Gameplay/Core/GameManager.h"
#include "Gameplay/Player.h"

ClientSocket::ClientSocket()
{
	s_pClient = this;

	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
	}
}

ClientSocket::~ClientSocket()
{
}

bool ClientSocket::ConnectToServer(const char* serverIp, int serverPort, const char* _pseudo, XMINT3 const& _color)
{
	inet_pton(AF_INET, serverIp, &m_serverAddr.sin_addr.s_addr);
	m_serverAddr.sin_family = AF_INET;
	m_serverAddr.sin_port = htons(serverPort);
	
	PacketHeader header;
	header.packetType = PacketType::ClientHello;

	MsgClientHello hello;
	hello.playerName = _pseudo;
	hello.r = _color.x;
	hello.g = _color.y;
	hello.b = _color.z;

	BufferWriter writer;
	SerializePacketHeader(writer, header);
	SerializeClientHello(writer, hello);

	SendPacket((const char*)writer.data.data(), (int)writer.data.size());

	m_gm = &GameManager::Get();

	m_isRunning = true;
	m_listenThread = CreateThread(NULL, 0, Listen, this, 0, NULL);
	return true;
}

DWORD ClientSocket::Listen(LPVOID lpParam)
{
	ClientSocket* pClient = static_cast<ClientSocket*>(lpParam);
	std::vector<uint8_t> buffer(MAX_UDP_PACKET_SIZE);

	while (pClient->m_isRunning)
	{
		socklen_t clientSocketLen = sizeof(sockaddr_in);

		int ret = recvfrom(
			pClient->m_socket,
			reinterpret_cast<char*>(buffer.data()),
			static_cast<int>(buffer.size()),
			0,
			reinterpret_cast<sockaddr*>(&pClient->m_serverAddr),
			&clientSocketLen
		);

		if (ret > 0)
		{
			BufferReader reader{ buffer.data(), static_cast<size_t>(ret), 0 };
			
			PacketHeader header;
			if (DeserializePacketHeader(reader, header) == false)
				continue;
			
			pClient->HandleOnPacketType(reader, header);
		}
		else if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (!pClient->m_isRunning)
				break;

			if (err == WSAECONNRESET)
			{
				continue; // ignore
			}

			if (err == WSAENOTSOCK || err == WSAEINVAL)
			{
				break;
			}

			std::cerr << "recvfrom failed, WSA error: " << err << "\n";
		}
	}

	return 0;
}

void ClientSocket::RecvPacket(std::vector<uint8_t> buffer, int length)
{
	sockaddr_in serverSocket;
	socklen_t serverSocketLen = sizeof(sockaddr_in);
	int ret = recvfrom(m_socket, (char*)buffer.data(), length, 0, (sockaddr*)&serverSocket, &serverSocketLen);

	if (ret > 0)
	{
		// Init d'un BufferReader pour lire les donn�es re�ues
		BufferReader reader{ buffer.data(), buffer.size(), 0 };
		PacketHeader header;
		UnpackHeader(reader, header);
		HandleOnPacketType(reader, header);
	}
	else
	{
		std::cout << "Erreur reception : " << WSAGetLastError() << std::endl;
	}
}

void ClientSocket::SendPacket(const char* data, int length)
{
	int ret = sendto(m_socket, (const char*)data, length, 0, (sockaddr*)&m_serverAddr, sizeof(m_serverAddr));
	if (ret == SOCKET_ERROR)
	{
		std::cout << "Erreur envoi : " << WSAGetLastError() << std::endl;
	}
}

void ClientSocket::UnpackHeader(BufferReader& reader, PacketHeader& header)
{
	bool headerOk = DeserializePacketHeader(reader, header);
}

void ClientSocket::HandleOnPacketType(BufferReader& reader, PacketHeader& header)
{
	switch (header.packetType)
	{
	case PacketType::ServerWelcome:
		HandleServerWelcome(reader);
		break;
	case PacketType::MatchStart:
		HandleMatchStart(reader);
		break;
	case PacketType::ServerSnapshot:
		if (header.sequence < m_lastSnapshotCount)
			break;
		HandleServerSnapshot(reader);
		break;
	case PacketType::NewMissile:
		HandleNewMissile(reader);
		break;
	case PacketType::DestroyMissile:
		HandleDestroyMissile(reader);
		break;
	case PacketType::PlayerInfo:
		HandlePlayersInfo(reader);
		break;
	case PacketType::PlayerJoin:
		HandlePlayerJoin(reader);
		break;
	case PacketType::Ping:
		HandlePing(reader);
		break;
	case PacketType::Pong:
		HandlePong(reader);
		break;
	case PacketType::Disconnect:
		HandleDisconnect(reader);
		break;
	default:
		break;
	}
}

void ClientSocket::HandleServerWelcome(BufferReader& reader)
{
	MsgServerWelcome decoded;
	bool msgOk = DeserializeServerWelcome(reader, decoded);
	m_isConnected = true;
			
	m_serverInfo.playerId = decoded.playerId;
	m_serverInfo.serverTickRate = decoded.serverTickRate;
	m_serverInfo.serverTimeMs = decoded.serverTimeMs;

	for (int i = 0; i < decoded.playerCount; i++)
	{
		if (i == decoded.playerId)
			m_gm->AddPlayer(XMFLOAT3(), 0.f, XMINT3(), "base", false, true);
		else
			m_gm->AddPlayer(XMFLOAT3(), 0.f, XMINT3(), "base", false);
	}
}

void ClientSocket::HandleMatchStart(BufferReader& reader)
{
	m_gm->SetMatchStart(true);
}

void ClientSocket::HandleServerSnapshot(BufferReader& reader)
{
	if (m_isConnected == false)
		return;

	MsgSnapshot decoded;
	bool msgOk = DeserializeServerSnapshot(reader, decoded);

	Player* player = m_gm->GetPlayers()[decoded.playerId];

	player->SetPosition(decoded.playerState.posX, decoded.playerState.posY, decoded.playerState.posZ);
	player->SetDirection(decoded.playerState.dirX, decoded.playerState.dirY, decoded.playerState.dirZ);
	player->GetLife() = decoded.playerState.hp;
	player->GetPlayerScore().SetScore(decoded.playerState.score);
}

void ClientSocket::HandleNewMissile(BufferReader& reader)
{
	MsgNewMissile decoded;
	bool msgOk = DeserializeNewMissile(reader, decoded);

	Player* shooter = m_gm->GetPlayers()[decoded.shooterID];
	shooter->Shoot();
}

void ClientSocket::HandleDestroyMissile(BufferReader& reader)
{
	MsgDestroyMissile decoded;
	bool msgOk = DeserializeDestroyMissile(reader, decoded);

	m_gm->SetToDestroyMissilesID(decoded.vMissileIDs);
}

void ClientSocket::HandlePlayerJoin(BufferReader& reader)
{
	m_gm->AddPlayer(XMFLOAT3(), 0.f, XMINT3(), "base", false);
}

void ClientSocket::HandlePlayersInfo(BufferReader& reader)
{
	MsgPlayerInfo decoded;
	bool msgOk = DeserializePlayerInfo(reader, decoded);

	m_gm->GetPlayers()[decoded.playerId]->SetPseudo(decoded.name);
	m_gm->GetPlayers()[decoded.playerId]->SetColor(decoded.r, decoded.g, decoded.b);
}

void ClientSocket::HandlePing(BufferReader& reader)
{
	std::cout << "Message Ping re�u\n";
}

void ClientSocket::HandlePong(BufferReader& reader)
{
	std::cout << "Message Pong re�u\n";
}

void ClientSocket::HandleDisconnect(BufferReader& reader)
{
	std::cout << "Message Disconnect re�u\n";
}

void ClientSocket::SendPing()
{
	if (m_isConnected == false)
		return;

	PacketHeader header;
	header.packetType = PacketType::Ping;
	header.playerId = m_serverInfo.playerId;
	BufferWriter writer;
	SerializePacketHeader(writer, header);

	SendPacket((const char*)writer.data.data(), (int)writer.data.size());
}
