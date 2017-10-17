#pragma once

#include <memory>
#include <queue>
#include <string>
#include <chrono>

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
public:
	NetworkClient(std::string address, unsigned short port, std::string username);
	~NetworkClient();

	std::shared_ptr <Packet> ReceivePacket();
	void SendPacket(std::shared_ptr<Packet> packet);

    void UpdatePacket();
};