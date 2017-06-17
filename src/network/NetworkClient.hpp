#pragma once

#include <thread>
#include <queue>
#include <mutex>
#include "Network.hpp"

class NetworkClient {
	Network network;
	std::thread networkThread;
	std::mutex toSendMutex;
	std::mutex toReceiveMutex;
	std::queue <std::shared_ptr<Packet>> toSend;
	std::queue <std::shared_ptr<Packet>> toReceive;
	bool isActive=true;
	bool &isRunning;
	ConnectionState state;
	void NetworkLoop();
public:
	NetworkClient(std::string address, unsigned short port, std::string username, bool &quit);
	~NetworkClient();

	std::shared_ptr <Packet> ReceivePacket();
	void SendPacket(std::shared_ptr<Packet> packet);
};