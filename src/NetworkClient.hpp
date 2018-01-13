#pragma once

#include <memory>
#include <queue>
#include <string>
#include <chrono>
#include <thread>

class Network;
struct Packet;
enum ConnectionState : unsigned char;

class NetworkClient {
    std::unique_ptr<Network> network;
	std::queue <std::shared_ptr<Packet>> toSend;
	std::queue <std::shared_ptr<Packet>> toReceive;
	ConnectionState state;
    int compressionThreshold = -1;
    std::chrono::steady_clock::time_point timeOfLastKeepAlivePacket;
	std::thread thread;
	bool isRunning=true;
	void ExecNs();
public:
	NetworkClient(std::string address, unsigned short port, std::string username);
	~NetworkClient();
};