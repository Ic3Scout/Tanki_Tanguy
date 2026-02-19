#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <string>
#include <vector>
#include <list>

#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <ws2tcpip.h>

// ============================
//  Constantes
// ============================
constexpr uint16_t PROTOCOL_MAGIC = 0xCAFE; // Identifie nos paquets
constexpr uint8_t  PROTOCOL_VERSION = 1;    // Change si tu casses la compat

// ============================
//  Types de paquets
// ============================
enum PacketType : uint8_t
{
	// Connection
	ClientHello = 0x01,			// Client -> Server
	ServerWelcome = 0x02,		// Server -> Client
	MatchStart = 0x03,			// Server -> Clients
	Disconnect = 0x04,			// Dans les 2 sens

	// Jeu
	ClientInput = 0x05,			// Client -> Server
	ServerSnapshot = 0x06,		// Server -> Client
	Ping = 0x07,				// Dans les 2 sens
	Pong = 0x08,				// Dans les 2 sens
	NewMissile = 0x09,			// Server -> Clients
	DestroyMissile = 0x10,		// Server -> Clients
	PlayerInfo = 0x11,			// Server -> Clients
	PlayerJoin = 0x12			// Server -> Clients
};

// ============================
//  Endianness helpers
//  Network order = Big Endian
// ============================
inline uint16_t HostToNet16(uint16_t hostValue) { return htons(hostValue); }
inline uint32_t HostToNet32(uint32_t hostValue) { return htonl(hostValue); }
inline uint16_t NetToHost16(uint16_t netValue) { return ntohs(netValue); }
inline uint32_t NetToHost32(uint32_t netValue) { return ntohl(netValue); }

inline bool IsLittleEndian()
{
	uint16_t x = 1;
	return *(uint8_t*)&x == 1;
}

inline uint64_t ByteSwap64(uint64_t x)
{
	return ((x & 0x00000000000000FFULL) << 56) |
		((x & 0x000000000000FF00ULL) << 40) |
		((x & 0x0000000000FF0000ULL) << 24) |
		((x & 0x00000000FF000000ULL) << 8) |
		((x & 0x000000FF00000000ULL) >> 8) |
		((x & 0x0000FF0000000000ULL) >> 24) |
		((x & 0x00FF000000000000ULL) >> 40) |
		((x & 0xFF00000000000000ULL) >> 56);
}

inline uint64_t HostToNet64(uint64_t value)
{
	if (IsLittleEndian())
		return ByteSwap64(value);
	return value;
}
inline uint64_t NetToHost64(uint64_t value)
{
	if (IsLittleEndian())
		return ByteSwap64(value);
	return value;
}

// ============================
//  SocketAddress
// ============================
struct SocketAddress
{
	sockaddr_in sockaddr{}; // IP + port

	bool operator==(SocketAddress& other)
	{
		bool sameFamiliy = sockaddr.sin_family == other.sockaddr.sin_family;
		bool samePort = sockaddr.sin_port == other.sockaddr.sin_port;
		bool sameAddress = sockaddr.sin_addr.s_addr == other.sockaddr.sin_addr.s_addr;

		return sameFamiliy && samePort && sameAddress;
	}
};

// ============================
//  En-t�te de packet (taille fixe)
// ============================
struct PacketHeader
{
	// --- ID ---
	uint16_t protocolMagic = PROTOCOL_MAGIC;		// PROTOCOL_MAGIC (network order)
	uint8_t  protocolVersion = PROTOCOL_VERSION;	// PROTOCOL_VERSION
	uint8_t  packetType = 0;						// PacketType (uint8_t)

	// --- Session routing ---
	uint8_t  playerId = -1;							// 0=unknown, 1=player1, 2=player2

	// --- Delivery / ordering ---
	uint16_t sequence = 0;							// Sequence number de CE paquet (network order)
	uint16_t ack = 0;								// Dernier sequence re�u du pair (network order)
	uint32_t ackBits = 0;							// Ack des 32 paquets pr�c�dents (network order)

	// --- Timing ---
	uint32_t timestampMs;							// Timestamp local (network order)
};

struct BufferWriter
{
	std::vector<uint8_t> data;

	void WriteU8(uint8_t hostValue)
	{
		data.push_back(hostValue);
	}

	void WriteU16(uint16_t hostValue)
	{
		hostValue = HostToNet16(hostValue);
		data.insert(data.end(), (uint8_t*)&hostValue, (uint8_t*)&hostValue + 2);
	}

	void WriteU32(uint32_t hostValue)
	{
		hostValue = HostToNet32(hostValue);
		data.insert(data.end(), (uint8_t*)&hostValue, (uint8_t*)&hostValue + 4);
	}

	void WriteU64(uint64_t hostValue)
	{
		hostValue = HostToNet64(hostValue);
		data.insert(data.end(), (uint8_t*)&hostValue, (uint8_t*)&hostValue + 8);
	}

	void WriteF32(float hostValue)
	{
		static_assert(sizeof(float) == 4, "float is not 32 bits");

		uint32_t bits;
		std::memcpy(&bits, &hostValue, 4);

		WriteU32(bits);
	}

	void WriteBytes(const void* src, size_t size)
	{
		const uint8_t* b = (const uint8_t*)src;
		data.insert(data.end(), b, b + size);
	}
};

struct BufferReader
{
	const uint8_t* data;
	size_t size;
	size_t offset = 0;

	bool CanRead(size_t n)
	{
		return offset + n <= size;
	}

	bool ReadU8(uint8_t& netValue)
	{
		if (CanRead(1) == false)
			return false;

		netValue = data[offset++];
		return true;
	}

	bool ReadU16(uint16_t& netValue)
	{
		if (CanRead(2) == false)
			return false;

		std::memcpy(&netValue, data + offset, 2);
		netValue = NetToHost16(netValue);
		offset += 2;
		return true;
	}

	bool ReadU32(uint32_t& netValue)
	{
		if (CanRead(4) == false)
			return false;

		std::memcpy(&netValue, data + offset, 4);
		netValue = NetToHost32(netValue);
		offset += 4;
		return true;
	}

	bool ReadU64(uint64_t& netValue)
	{
		if (CanRead(8) == false)
			return false;

		std::memcpy(&netValue, data + offset, 8);
		netValue = NetToHost64(netValue);
		offset += 8;
		return true;
	}

	bool ReadF32(float& netValue)
	{
		uint32_t bits;
		if (ReadU32(bits) == false)
			return false;

		std::memcpy(&netValue, &bits, 4);
		return true;
	}

	bool ReadBytes(void* dst, size_t n)
	{
		if (CanRead(n) == false)
			return false;

		std::memcpy(dst, data + offset, n);
		offset += n;
		return true;
	}
};

constexpr size_t PACKET_HEADER_SIZE = 17;

inline void SerializePacketHeader(BufferWriter& writer, const PacketHeader& header)
{
	writer.WriteU16(header.protocolMagic);
	writer.WriteU8(header.protocolVersion);
	writer.WriteU8(header.packetType);
	writer.WriteU8(header.playerId);
	writer.WriteU16(header.sequence);
	writer.WriteU16(header.ack);
	writer.WriteU32(header.ackBits);
	writer.WriteU32(header.timestampMs);
}

inline bool DeserializePacketHeader(BufferReader& reader, PacketHeader& header)
{
	uint8_t type;
	if (reader.ReadU16(header.protocolMagic) == false)		return false;
	if (reader.ReadU8(header.protocolVersion) == false)		return false;
	if (reader.ReadU8(type) == false)						return false;
	header.packetType = type;
	if (reader.ReadU8(header.playerId) == false)			return false;
	if (reader.ReadU16(header.sequence) == false)			return false;
	if (reader.ReadU16(header.ack) == false)				return false;
	if (reader.ReadU32(header.ackBits) == false)			return false;
	if (reader.ReadU32(header.timestampMs) == false)		return false;

	return header.protocolMagic == PROTOCOL_MAGIC && header.protocolVersion == PROTOCOL_VERSION;
}

struct MsgClientHello
{
	std::string playerName;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

inline void SerializeClientHello(BufferWriter& writer, const MsgClientHello& msg)
{
	writer.WriteU8((uint8_t)msg.playerName.size());
	writer.WriteBytes(msg.playerName.data(), msg.playerName.size());
	writer.WriteU8(msg.r);
	writer.WriteU8(msg.g);
	writer.WriteU8(msg.b);
}

inline bool DeserializeClientHello(BufferReader& reader, MsgClientHello& msg)
{
	uint8_t nameLen;
	if (reader.ReadU8(nameLen) == false)			return false;
	if (reader.CanRead(nameLen) == false)			return false;

	msg.playerName.resize(nameLen);
	if (nameLen == 0)
		return true;

	if (reader.ReadBytes(&msg.playerName[0], nameLen) == false)
		return false;

	if (reader.ReadU8(msg.r) == false)
		return false;
	if (reader.ReadU8(msg.g) == false)
		return false;
	if (reader.ReadU8(msg.b) == false)
		return false;
	return true;
}

struct MsgServerWelcome
{
	uint8_t  playerId;
	uint8_t	 playerCount;
	uint8_t  serverTickRate;
	uint32_t serverTimeMs;
};

inline void SerializeServerWelcome(BufferWriter& writer, const MsgServerWelcome& message)
{
	writer.WriteU8(message.playerId);
	writer.WriteU8(message.playerCount);
	writer.WriteU8(message.serverTickRate);
	writer.WriteU32(message.serverTimeMs);
}

inline bool DeserializeServerWelcome(BufferReader& reader, MsgServerWelcome& message)
{
	return reader.ReadU8(message.playerId) &&
		reader.ReadU8(message.playerCount) &&
		reader.ReadU8(message.serverTickRate) &&
		reader.ReadU32(message.serverTimeMs);
}

struct InputCmd
{
	uint16_t buttons;		// actions 
};

struct MsgInputList
{
	std::vector<InputCmd> inputsList;
};

inline void SerializeInputList(BufferWriter& writer, const MsgInputList& msg)
{
	uint16_t count = static_cast<uint16_t>(msg.inputsList.size());
	writer.WriteU16(count);

	for (const auto& input : msg.inputsList)
	{
		writer.WriteU16(input.buttons);
	}
}

inline bool DeserializeInputList(BufferReader& reader, MsgInputList& msg)
{
	uint16_t count;
	if (reader.ReadU16(count) == false)
		return false;

	msg.inputsList.clear();
	msg.inputsList.reserve(count);

	for (uint16_t i = 0; i < count; ++i)
	{
		InputCmd cmd;
		if (reader.ReadU16(cmd.buttons) == false)
			return false;

		msg.inputsList.push_back(cmd);
	}

	return true;
}

struct PlayerNetState
{
	float posX = 0;
	float posY = 0;
	float posZ = 0;
	float dirX = 0;
	float dirY = 0;
	float dirZ = 0;
	uint16_t hp = 0;
	uint64_t score = 0;
};

inline void SerializePlayerNetState(BufferWriter& writer, const PlayerNetState& playerNetState)
{
	writer.WriteF32(playerNetState.posX);
	writer.WriteF32(playerNetState.posY);
	writer.WriteF32(playerNetState.posZ);
	writer.WriteF32(playerNetState.dirX);
	writer.WriteF32(playerNetState.dirY);
	writer.WriteF32(playerNetState.dirZ);
	writer.WriteU16(playerNetState.hp);
	writer.WriteU64(playerNetState.score);
}

inline bool DeserializePlayerNetState(BufferReader& reader, PlayerNetState& playerNetState)
{
	return reader.ReadF32(playerNetState.posX) &&
		reader.ReadF32(playerNetState.posY) &&
		reader.ReadF32(playerNetState.posZ) &&
		reader.ReadF32(playerNetState.dirX) &&
		reader.ReadF32(playerNetState.dirY) &&
		reader.ReadF32(playerNetState.dirZ) &&
		reader.ReadU16(playerNetState.hp) &&
		reader.ReadU64(playerNetState.score);
}

struct MsgPlayerInfo
{
	uint8_t playerId;
	std::string	name;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

inline void SerializePlayerInfo(BufferWriter& writer, const MsgPlayerInfo& p)
{
	writer.WriteU8(p.playerId);
	writer.WriteU8((uint8_t)p.name.size());
	writer.WriteBytes(p.name.data(), p.name.size());
	writer.WriteU8(p.r);
	writer.WriteU8(p.g);
	writer.WriteU8(p.b);
}

inline bool DeserializePlayerInfo(BufferReader& reader, MsgPlayerInfo& p)
{
	if (reader.ReadU8(p.playerId) == false)			return false;
	uint8_t nameLen;
	if (reader.ReadU8(nameLen) == false)			return false;
	if (reader.CanRead(nameLen) == false)			return false;

	p.name.resize(nameLen);

	if (reader.ReadBytes(&p.name[0], nameLen) == false)
		return false;

	if (reader.ReadU8(p.r) == false)
		return false;

	if (reader.ReadU8(p.g) == false)
		return false;

	if (reader.ReadU8(p.b) == false)
		return false;

	return true;
}


struct MsgSnapshot
{
	uint8_t playerId = 0;
	PlayerNetState playerState;
};

inline void SerializeServerSnapshot(BufferWriter& writer, const MsgSnapshot& msg)
{
	writer.WriteU8(msg.playerId);
	SerializePlayerNetState(writer, msg.playerState);
	
}

inline bool DeserializeServerSnapshot(BufferReader& reader, MsgSnapshot& msg)
{
	return reader.ReadU8(msg.playerId) &&
		DeserializePlayerNetState(reader, msg.playerState);
}

struct MsgPing
{
	uint32_t pingId;
};

inline void SerializePing(BufferWriter& writer, const MsgPing& msg)
{
	writer.WriteU32(msg.pingId);
}

inline bool DeserializePing(BufferReader& reader, MsgPing& msg)
{
	return reader.ReadU32(msg.pingId);
}

struct MsgNewMissile
{
	uint8_t shooterID;
};

inline void SerializeNewMissile(BufferWriter& writer, const MsgNewMissile& msg)
{
	writer.WriteU8(msg.shooterID);
}

inline bool DeserializeNewMissile(BufferReader& reader, MsgNewMissile& msg)
{
	return reader.ReadU8(msg.shooterID);
}

struct MsgDestroyMissile
{
	std::vector<uint16_t> vMissileIDs;
};

inline void SerializeDestroyMissile(BufferWriter& writer, const MsgDestroyMissile& msg)
{
	uint16_t count = static_cast<uint16_t>(msg.vMissileIDs.size());
	writer.WriteU16(count);

	for (const auto& id : msg.vMissileIDs)
	{
		writer.WriteU16(id);
	}
}

inline bool DeserializeDestroyMissile(BufferReader& reader, MsgDestroyMissile& msg)
{
	uint16_t count;
	if (reader.ReadU16(count) == false)		return false;
	
	msg.vMissileIDs.clear();
	msg.vMissileIDs.reserve(count);

	for (uint16_t i = 0; i < count; ++i)
	{
		uint16_t id;
		if (reader.ReadU16(id) == false)	return false;
			
		msg.vMissileIDs.push_back(id);
	}

	return true;
}

#endif 