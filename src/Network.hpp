#pragma once

#include <memory>

#include "Packet.hpp"
#include "platform/StreamSocket.hpp"

enum ConnectionState : unsigned char {
	Handshaking,
	Login,
	Play,
	Status,
};

class Network {
	std::unique_ptr<StreamSocket> stream;

	std::shared_ptr<Packet> ReceivePacketByPacketId(int packetId, ConnectionState state, StreamInput &stream);
public:
	Network(std::string address, unsigned short port);
	void Connect();

	std::shared_ptr<Packet> ReceivePacket(ConnectionState state = Play, bool useCompression = false);
	void SendPacket(Packet &packet, int compressionThreshold = -1, bool more = false);
	std::shared_ptr<Packet> ParsePacketPlay(PacketNamePlayCB id);
};
