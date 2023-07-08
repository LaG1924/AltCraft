#pragma once

#include <memory>
#include "Packet.hpp"

enum ConnectionState : unsigned char {
	Handshaking,
	Login,
	Play,
	Status,
};

class Network {
	std::unique_ptr<Socket> socket;
	std::unique_ptr<StreamSocket> stream;

	std::shared_ptr<Packet> ReceivePacketByPacketId(int packetId, ConnectionState state, StreamInput &in);
public:
	Network(std::string address, unsigned short port);

	Network(const Network&) = delete;

	Network(Network&&) = default;

	Network& operator=(const Network&) = delete;
	Network& operator=(Network&&) = default;

	~Network();

	std::shared_ptr<Packet> ReceivePacket(ConnectionState state = Play, bool useCompression = false);
	void SendPacket(Packet &packet, int compressionThreshold = -1);
	std::shared_ptr<Packet> ParsePacketPlay(PacketNamePlayCB id);
};