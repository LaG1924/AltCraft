#pragma once

#include <memory>

#include "Packet.hpp"
#include "platform/Socket.hpp"

enum ConnectionState : unsigned char {
	Handshaking,
	Login,
	Play,
	Status,
};

class Network {
	std::unique_ptr<Socket> socket;

	uint32_t ReadPacketLen();
	std::shared_ptr<Packet> ReceivePacketByPacketId(int packetId, ConnectionState state, StreamInput &stream);
public:
	Network(std::string address, unsigned short port);
	void Connect(unsigned char *buffPtr, size_t buffLen);

	std::shared_ptr<Packet> ReceivePacket(ConnectionState state = Play, bool useCompression = false);
	void SendPacket(Packet &packet, int compressionThreshold = -1, bool more = false);
	std::shared_ptr<Packet> ParsePacketPlay(PacketNamePlayCB id);
};
