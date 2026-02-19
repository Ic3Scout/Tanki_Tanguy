#include "Server.h"
#include "Gameplay/Player.h"
#include "Gameplay/PlayerController.h"
#include "Gameplay/Core/GameManager.h"

Server::Server()
{
	s_pInstance = this;
	m_socket = NULL;
	std::memset(&m_lastClientAddr, 0, sizeof(m_lastClientAddr)); // Initialisation explicite
}

Server::~Server()
{
	Exit();
}

void Server::MainLoop()
{
	while (m_running)
	{
		Update();
	}
}

void Server::Start()
{
	InitializeSocket();

	m_running = true;
	m_pGM = new GameManager(true);
	m_pGM->ServerSideStart();
	m_listenThread = CreateThread(NULL, 0, Listen, this, 0, NULL);
}

void Server::Update()
{
	m_pGM->ServerSideUpdate();
	CheckClientsConnected();
	SendNewMissile();
	SendMissilesToDestroy();
	SendServerSnapshotsToClients();	
	Sleep(16);
}

void Server::Exit()
{
	m_running = false;

	// fermer le socket r�veille recvfrom
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	for (int i = 0; i < m_vClientsInfo.size(); ++i)
	{
		delete m_vClientsInfo[i];
	}	

	if (m_listenThread)
	{
		WaitForSingleObject(m_listenThread, INFINITE);
		CloseHandle(m_listenThread);
		m_listenThread = nullptr;
	}
	if (m_pGM)
	{
		//m_pGM->ServerSideStop();
	}
}

void Server::InitializeSocket()
{
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (m_socket == INVALID_SOCKET)
	{
		std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
	}

	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(1888);

	if (bind(m_socket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
	}
}

DWORD Server::Listen(LPVOID lpParam)
{
	Server* pServer = static_cast<Server*>(lpParam);
	std::vector<uint8_t> buffer(MAX_UDP_PACKET_SIZE);

	while (pServer->m_running)
	{
		socklen_t clientSocketLen = sizeof(sockaddr_in);

		int ret = recvfrom(
			pServer->m_socket,
			reinterpret_cast<char*>(buffer.data()),
			static_cast<int>(buffer.size()),
			0,
			reinterpret_cast<sockaddr*>(&pServer->m_lastClientAddr),
			&clientSocketLen
		);

		if (ret > 0)
		{
			BufferReader reader { buffer.data(), static_cast<size_t>(ret), 0 };

			PacketHeader header;
			if (DeserializePacketHeader(reader, header) == false) 
				continue;
			if (header.packetType != PacketType::ClientHello)
				pServer->m_vClientsInfo[header.playerId]->lastHeardTime = 0.f;

			pServer->HandleOnPacketType(reader, header);
		}
		else if (ret == SOCKET_ERROR)
		{
			int err = WSAGetLastError();

			if (!pServer->m_running) 
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

void Server::SendPacket(uint8_t clientID, const char* data, int length)
{
	ClientInfo* pClient = m_vClientsInfo[clientID];

	if (pClient == nullptr || pClient->isConnected == false)
	{
		std::cerr << "Attempted to send packet to invalid or disconnected client ID: " << (int)clientID << "\n";
		return;
	}
	sockaddr_in clientAddr = pClient->address.sockaddr;
	int sentBytes = sendto(
		m_socket,
		data,
		length,
		0,
		(sockaddr*)&clientAddr,
		sizeof(clientAddr)
	);

	if (sentBytes == SOCKET_ERROR)
		std::cerr << "sendto failed with error: " << WSAGetLastError() << "\n";
}

void Server::CheckClientsConnected()
{
	for (ClientInfo* client : m_vClientsInfo)
	{
		if (client->isConnected)
		{
			client->lastHeardTime += m_pGM->GetDeltaTime();
			if (client->lastHeardTime >= TIMEOUT)
			{
				client->isConnected = false;
				std::cout << "client deco" << std::endl;
			}
		}
	}
}

void Server::UnpackHeader(BufferReader& reader, PacketHeader& header)
{
	bool headerOk = DeserializePacketHeader(reader, header);

	if (headerOk)
		std::cout << "Packet Type: " << (int)header.packetType << "\n";
	else
		std::cout << "Header invalide re�u\n";
}

void Server::HandleOnPacketType(BufferReader& reader, PacketHeader& header)
{
	switch (header.packetType)
	{
	case PacketType::ClientHello:
		HandleClientHello(reader);
		break;
	case PacketType::ClientInput:
		HandleClientInput(reader, header);
		break;
	case PacketType::Ping:
		HandlePing(reader, header);
		break;
	case PacketType::Pong:
		HandlePong(reader, header);
		break;
	case PacketType::Disconnect:
		HandleDisconnect(reader, header);
		break;
	default:
		std::cout << "Type de message inconnu re�u: " << (int)header.packetType << "\n";
		break;
	}
}

void Server::HandleClientHello(BufferReader& reader)
{
	MsgClientHello decoded;
	bool msgOk = DeserializeClientHello(reader, decoded);

	bool createNew = true;
	for (int clientIndex = 0; clientIndex < m_vClientsInfo.size(); clientIndex++)
	{
		if (m_vClientsInfo[clientIndex]->isConnected)
			continue;
		else if (m_vClientsInfo[clientIndex]->playerName == decoded.playerName)
		{
			SetClientInfo(m_vClientsInfo[clientIndex], (SocketAddress)m_lastClientAddr, decoded);
			SendWelcomeMsg(clientIndex);
			createNew = false;
		}
	}
	if (createNew)
		AddNewClient(decoded);

	SendPlayersInfo();
}

void Server::HandleClientInput(BufferReader& reader, PacketHeader& header)
{
	MsgInputList decoded;
	bool msgOk = DeserializeInputList(reader, decoded);
	if (msgOk == false) return;

	int playerIndex = header.playerId;
	
	for (const InputCmd& cmd : decoded.inputsList)
	{
		SetPlayerInput(playerIndex, cmd.buttons);
	}
}

void Server::HandlePing(BufferReader& reader, PacketHeader& header)
{

}

void Server::HandlePong(BufferReader& reader, PacketHeader& header)
{

}

void Server::HandleDisconnect(BufferReader& reader, PacketHeader& header)
{
	std::cout << "Player " << (int)header.playerId << " disconnected" << std::endl;
	m_vClientsInfo[header.playerId]->isConnected = false;
}

void Server::SendMatchStart()
{
	PacketHeader header;
	header.packetType = PacketType::MatchStart;
	BufferWriter writer;
	SerializePacketHeader(writer, header);
	
	for (int clientID = 0; clientID < m_vClientsInfo.size(); clientID++)
	{
		SendPacket(clientID, (const char*)writer.data.data(), (int)writer.data.size());
	}
}

void Server::SendNewMissile()
{
	if (m_pGM->GetNewMissileAdded() == false)
		return;

	PacketHeader header;
	header.packetType = PacketType::NewMissile;

	MsgNewMissile msg;
	msg.shooterID = m_pGM->GetLastMissileAdded()->GetShooterId();
	BufferWriter writer;
	SerializePacketHeader(writer, header);
	SerializeNewMissile(writer, msg);

	for (int clientID = 0; clientID < m_vClientsInfo.size(); clientID++)
	{
		SendPacket(clientID, (const char*)writer.data.data(), (int)writer.data.size());
	}
	m_pGM->SetNewMissileAdded(false);
}

void Server::SendPlayersInfo()
{
	PacketHeader header;
	header.packetType = PacketType::PlayerInfo;
	
	for (int clientID = 0; clientID < m_vClientsInfo.size(); clientID++)
	{
		for (int otherClientID = 0; otherClientID < m_vClientsInfo.size(); otherClientID++)
		{
			if (clientID == otherClientID) continue;

			MsgPlayerInfo msg;
			msg.playerId = otherClientID;
			msg.name = m_vClientsInfo[otherClientID]->playerName;
			msg.r = m_vClientsInfo[otherClientID]->playerRed;
			msg.g = m_vClientsInfo[otherClientID]->playerGreen;
			msg.b = m_vClientsInfo[otherClientID]->playerBlue;

			BufferWriter writer;
			SerializePacketHeader(writer, header);
			SerializePlayerInfo(writer, msg);
			SendPacket(clientID, (const char*)writer.data.data(), (int)writer.data.size());
		}
	}
}

void Server::SendMissilesToDestroy()
{
	if (m_pGM->GetToDestroyMissilesID().empty())
		return;

	PacketHeader header;
	header.packetType = PacketType::DestroyMissile;

	MsgDestroyMissile msg;
	msg.vMissileIDs = m_pGM->GetToDestroyMissilesID();

	m_pGM->GetToDestroyMissilesID().clear();

	BufferWriter writer;
	SerializePacketHeader(writer, header);
	SerializeDestroyMissile(writer, msg);

	for (int clientID = 0; clientID < m_vClientsInfo.size(); clientID++)
	{
		SendPacket(clientID, (const char*)writer.data.data(), (int)writer.data.size());
	}
}

void Server::SendNewPlayer()
{
	for (int clientID = 0; clientID < m_vClientsInfo.size() - 1; clientID++)
	{
		PacketHeader header;
		header.packetType = PacketType::PlayerJoin;
		BufferWriter writer;
		SerializePacketHeader(writer, header);
		SendPacket(clientID, (const char*)writer.data.data(), (int)writer.data.size());
	}
}

void Server::SendServerSnapshotsToClients()
{
	PacketHeader header;
	header.packetType = PacketType::ServerSnapshot;
	header.sequence = m_snapshotCount;

	for (int clientID = 0; clientID < m_vClientsInfo.size(); clientID++)
	{
		if (m_vClientsInfo[clientID]->isConnected == false) continue;
		//Pour chaque client envoie des infos des autres players
		for (int playerID = 0; playerID < m_pGM->GetPlayers().size(); playerID++)
		{
			Player* player = m_pGM->GetPlayers()[playerID];

			if (player->IsDead())
				player->Dead(m_pGM->GetDeltaTime());
			
			PlayerNetState playerState;
			playerState.posX = player->GetPos().x;
			playerState.posY = player->GetPos().y;
			playerState.posZ = player->GetPos().z;
			playerState.dirX = player->GetDir().x;
			playerState.dirY = player->GetDir().y;
			playerState.dirZ = player->GetDir().z;
			playerState.hp = player->GetLife().GetCurrentLife();
			playerState.score = player->GetScore();

			MsgSnapshot snapshotMsg;
			snapshotMsg.playerId = playerID;
			snapshotMsg.playerState = playerState;

			BufferWriter writer;
			SerializePacketHeader(writer, header);
			SerializeServerSnapshot(writer, snapshotMsg);
			SendPacket(clientID, (const char*)writer.data.data(), (int)writer.data.size());
		}
	}
	m_snapshotCount++;
}

void Server::SendWelcomeMsg(uint8_t _clientID)
{
	PacketHeader header;
	header.packetType = PacketType::ServerWelcome;

	MsgServerWelcome welcomeMsg;
	welcomeMsg.playerId = _clientID;
	welcomeMsg.playerCount = m_playerCount;
	welcomeMsg.serverTickRate = 60;
	welcomeMsg.serverTimeMs = 0;

	BufferWriter writer;
	SerializePacketHeader(writer, header);
	SerializeServerWelcome(writer, welcomeMsg);
	SendPacket(welcomeMsg.playerId, (const char*)writer.data.data(), (int)writer.data.size());
}

void Server::AddNewClient(MsgClientHello& msg)
{
	m_playerCount++;
	ClientInfo* newClient = new ClientInfo();
	newClient->playerId = m_vClientsInfo.size();
	m_vClientsInfo.push_back(newClient);

	m_pGM->AddPlayer(XMFLOAT3(), 0.f, XMINT3(), newClient->playerName, true);
	m_pGM->RespawnPlayer(m_pGM->GetPlayers()[newClient->playerId]);
	SetClientInfo(m_vClientsInfo[newClient->playerId], (SocketAddress)m_lastClientAddr, msg);

	SendWelcomeMsg(newClient->playerId);
	SendNewPlayer();
}

void Server::SetPlayerInput(int playerIndex, uint16_t buttons)
{
	GameManager::Get().GetPlayerControllers()[playerIndex]->AddIncomingInputs(buttons);
	std::cout << "Received input from Player " << (playerIndex + 1) << ": " << buttons << "\n";
}

void Server::SetClientInfo(ClientInfo* pInfo, const SocketAddress& addr, const MsgClientHello& msg)
{
	pInfo->playerName	= msg.playerName;
	pInfo->playerRed	= msg.r;
	pInfo->playerGreen	= msg.g;
	pInfo->playerBlue	= msg.b;
	pInfo->address		= addr;
	pInfo->isConnected	= true;
	m_pGM->GetPlayers()[pInfo->playerId]->SetPseudo(msg.playerName);
	m_pGM->GetPlayers()[pInfo->playerId]->SetColor(msg.r, msg.g, msg.b);
}
