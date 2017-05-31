#include "NetworkClient.hpp"

ServerInfo NetworkClient::ServerPing(std::string address, unsigned short port) {
    ServerInfo info;
    Network network(address, port);
    Packet packet_handshake = PacketBuilder::CHandshaking0x00(316, address, port, 1);
    network.SendPacket(packet_handshake);
    Packet packet_request(0);
    network.SendPacket(packet_request);
    Packet packet_response = network.ReceivePacket();
    PacketParser::Parse(packet_response, Login);
    //std::string json = static_cast<FieldString *>(packet_response_parsed.GetFieldById(0))->GetValue();
    std::string json = packet_response.GetField(0).GetString();
    try {
        nlohmann::json j = nlohmann::json::parse(json);
        info.protocol = j["version"]["protocol"].get<int>();
        info.version = j["version"]["name"].get<std::string>();
        info.players_max = j["players"]["max"].get<int>();
        info.players_online = j["players"]["online"].get<int>();
        info.description = j["description"]["text"].get<std::string>();
        for (auto t:j["description"]["extra"]) {
            info.description += t["text"].get<std::string>();
        }
        if (!j["favicon"].is_null())
            info.favicon = j["favicon"].get<std::string>();
        info.json = json;
        for (auto t:j["players"]["sample"]) {
            std::pair<std::string, std::string> player;
            player.first = t["id"].get<std::string>();
            player.second = t["name"].get<std::string>();
            info.players.push_back(player);
        }
    } catch (const nlohmann::detail::exception e) {
        std::cerr << "Parsed json is not valid (" << e.id << "): " << e.what() << std::endl;
    }
    //Ping
    Packet packet_ping(0x01);
    Field payload;
    payload.SetLong(771235);
    packet_ping.AddField(payload);
    std::chrono::high_resolution_clock clock;
    auto t1 = clock.now();
    network.SendPacket(packet_ping);
    Packet pong = network.ReceivePacket();
    auto t2 = clock.now();
    pong.ParseField(Long);
    if (pong.GetField(0).GetLong() == 771235) {
        std::chrono::duration<double, std::milli> pingTime = t2 - t1;
        info.ping = pingTime.count();
    }
    return info;
}

NetworkClient::NetworkClient(std::string address, unsigned short port, std::string username) : m_network(address,
                                                                                                         port) {
    m_network.SendHandshake(username);
    Update();
    m_networkThread = std::thread(&NetworkClient::MainLoop, this);
}

NetworkClient::~NetworkClient() {
    LOG(INFO)<<"NC stopping...";
    isContinue=false;
    m_networkThread.join();
    LOG(INFO)<<"NC is stopped";
}

Packet * NetworkClient::GetPacket() {
    if (m_received.size() < 1)
        return nullptr;
    Packet packet = m_received.front();
    m_received.pop();
    return new Packet(packet);
}

void NetworkClient::AddPacketToQueue(Packet packet) {
    m_toSend.push(packet);
}

void NetworkClient::Update() {
    if (m_toSend.size() > 0) {
        m_network.SendPacket(m_toSend.front());
        m_toSend.pop();
    }
    Packet received = m_network.ReceivePacket();
    if (received.GetId() == 0x1F) {
        PacketParser::Parse(received);
        Packet response = PacketBuilder::CPlay0x0B(received.GetField(0).GetVarInt());
        m_network.SendPacket(response);
        return;
    }
    m_updateMutex.lock();
    m_received.push(received);
    m_updateMutex.unlock();
}

void NetworkClient::MainLoop() {
    el::Helpers::setThreadName("Network");
    try {
        while (isContinue) {
            Update();
        }
    } catch (int e){
        LOG(ERROR)<<"Catched exception in NC: "<<e;
    }

}
