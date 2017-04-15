#pragma once

#include <queue>
#include "Network.hpp"

struct ServerInfo{
    std::string version;
    int protocol = 0;
    int players_max = 0;
    int players_online = 0;
    std::vector<std::pair<std::string, std::string>> players;
    std::string description;
    double ping = 0;
    std::string favicon;
    std::string json;
};
class NetworkClient {
public:
    NetworkClient(std::string address, unsigned short port, std::string username);
    ~NetworkClient();

    void Update();

    Packet GetPacket();
    void AddPacketToQueue(Packet packet);

    static ServerInfo ServerPing(std::string address,unsigned short port);
private:
    NetworkClient (const NetworkClient&);
    NetworkClient&operator=(const NetworkClient&);
    Network m_network;
    std::queue <Packet> m_received;
    std::queue <Packet> m_toSend;
};

