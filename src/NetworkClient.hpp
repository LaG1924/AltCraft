#pragma once

#include <thread>
#include <queue>
#include <mutex>

#include "Network.hpp"
#include "Event.hpp"

class NetworkClient {
	Network network;
	std::thread networkThread;
	std::mutex toSendMutex;
	std::mutex toReceiveMutex;
	std::queue <std::shared_ptr<Packet>> toSend;
	std::queue <std::shared_ptr<Packet>> toReceive;
	bool isActive=true;
	ConnectionState state;
	void NetworkLoop();
    int compressionThreshold = -1;
public:
	NetworkClient(std::string address, unsigned short port, std::string username);
	~NetworkClient();

	std::shared_ptr <Packet> ReceivePacket();
	void SendPacket(std::shared_ptr<Packet> packet);
};