#pragma once

#include <queue>
#include <thread>
#include <mutex>
#include <nlohmann/json.hpp>
#include "Network.hpp"
#include "../packet/PacketParser.hpp"
#include "../packet/PacketBuilder.hpp"

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

    void MainLoop();

    Packet * GetPacket();
    void AddPacketToQueue(Packet packet);

    static ServerInfo ServerPing(std::string address,unsigned short port);
private:
    std::mutex m_updateMutex;
    std::thread m_networkThread;
    bool isContinue=true;
    NetworkClient (const NetworkClient&);
    NetworkClient&operator=(const NetworkClient&);
    Network m_network;
    std::queue <Packet> m_received;
    std::queue <Packet> m_toSend;
};

