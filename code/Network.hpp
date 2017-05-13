#pragma once

#include <string>
#include <SFML/Network.hpp>
#include "Packet.hpp"


class Network {
public:
    Network(std::string address, unsigned short port);

    ~Network();

    void SendHandshake(std::string username);

    void SendPacket(Packet &packet);

    Packet ReceivePacket();

private:
    std::string m_address;
    unsigned short m_port;
    sf::TcpSocket m_socket;
    bool m_isCommpress=false;
};

