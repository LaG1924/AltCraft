#pragma once

#include <memory>
#include "Socket.hpp"
#include "Packet.hpp"

enum ConnectionState {
	Handshaking,
	Login,
	Play,
	Status,
};

class Network {
	Socket *socket = nullptr;
	StreamSocket *stream = nullptr;

	std::shared_ptr<Packet> ReceivePacketByPacketId(int packetId, ConnectionState state, StreamInput &stream);
public:
	Network(std::string address, unsigned short port);
	~Network();

	std::shared_ptr<Packet> ReceivePacket(ConnectionState state = Play);
	void SendPacket(Packet &packet);
	std::shared_ptr<Packet> ParsePacketPlay(PacketNamePlayCB id);
};