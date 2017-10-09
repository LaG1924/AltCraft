#pragma once

#include <thread>
#include <queue>
#include <mutex>

#include "Network.hpp"
#include "Event.hpp"

class NetworkClient {
	Network network;
	std::queue <std::shared_ptr<Packet>> toSend;
	std::queue <std::shared_ptr<Packet>> toReceive;
	ConnectionState state;
    int compressionThreshold = -1;
    std::chrono::steady_clock::time_point timeOfLastKeepAlivePacket;
public:
	NetworkClient(std::string address, unsigned short port, std::string username);
	~NetworkClient();

	std::shared_ptr <Packet> ReceivePacket();
	void SendPacket(std::shared_ptr<Packet> packet);

    void UpdatePacket();
};