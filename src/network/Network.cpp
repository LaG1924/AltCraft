#include "Network.hpp"

Network::Network(std::string address, unsigned short port) : m_address(address), m_port(port) {
    LOG(INFO) << "Connecting to server " << m_address << ":" << m_port;
    sf::Socket::Status status = m_socket.connect(sf::IpAddress(m_address), m_port);
    m_socket.setBlocking(true);
    if (status != sf::Socket::Done) {
        if (status == sf::Socket::Error) {
            LOG(ERROR) << "Can't connect to remote server";
        } else {
            LOG(ERROR) << "Connection failed with unknown reason";
			throw std::runtime_error("Connection is failed");
            throw 13;
        }
    }
    LOG(INFO) << "Connected to server";
}

Network::~Network() {
    m_socket.disconnect();
    LOG(INFO) << "Disconnected";
}

void Network::SendHandshake(std::string username) {
    //Handshake packet
    Packet handshakePacket = PacketBuilder::CHandshaking0x00(316, m_address, m_port, 2);
    SendPacket(handshakePacket);

    //LoginStart packet
    Field fName;
    fName.SetString(username);
    Packet loginPacket(0);
    loginPacket.AddField(fName);
    SendPacket(loginPacket);
}

void Network::SendPacket(Packet &packet) {
    m_socket.setBlocking(true);
    byte *packetData = new byte[packet.GetLength()];
    packet.CopyToBuff(packetData);
    m_socket.send(packetData, packet.GetLength());
    delete[] packetData;
}

Packet Network::ReceivePacket() {
    byte bufLen[5] = {0};
    size_t rec = 0;
    for (int i = 0; i < 5; i++) {
        byte buff = 0;
        size_t r = 0;
        m_socket.receive(&buff, 1, r);
        rec += r;
        bufLen[i] = buff;
        if ((buff & 0b10000000) == 0) {
            break;
        }
    }
    Field fLen = FieldParser::Parse(VarIntType, bufLen);
    size_t packetLen = fLen.GetVarInt() + fLen.GetLength();
    if (packetLen > 1024 * 1024 * 15)
        LOG(WARNING)<<"OMG SIZEOF PACKAGE IS "<<packetLen;
    if (packetLen < rec) {
        return Packet(bufLen);
    }
    byte *bufPack = new byte[packetLen];
    std::copy(bufLen, bufLen + rec, bufPack);
    size_t dataLen = rec;
    while (m_socket.receive(bufPack + dataLen, packetLen - dataLen, rec) == sf::Socket::Done && dataLen < packetLen) {
        dataLen += rec;
    }
    if (dataLen < packetLen) {
        LOG(ERROR) << "Received data is "<<dataLen<<" but "<<packetLen<<" is promoted";
        throw std::runtime_error("Data is losted");
    } else {
        Packet p(bufPack);
        delete[] bufPack;
        return p;
    }
}
