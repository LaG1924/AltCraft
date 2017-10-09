#include "NetworkClient.hpp"

NetworkClient::NetworkClient(std::string address, unsigned short port, std::string username)
		: network(address, port) {
	state = Handshaking;

	PacketHandshake handshake;
	handshake.protocolVersion = 338;
	handshake.serverAddress = address;
	handshake.serverPort = port;
	handshake.nextState = 2;
	network.SendPacket(handshake);
	state = Login;

	PacketLoginStart loginStart;
	loginStart.Username = "HelloOne";
	network.SendPacket(loginStart);


    auto packet = network.ReceivePacket(Login);

    while (!packet)
        packet = network.ReceivePacket(Login);

    if (packet->GetPacketId() == PacketNameLoginCB::SetCompression) {
        auto compPacket = std::static_pointer_cast<PacketSetCompression>(packet);
        LOG(INFO) << "Compression threshold: " << compPacket->Threshold;
        compressionThreshold = compPacket->Threshold;
        packet.reset();
        while (!packet)
            packet = network.ReceivePacket(Login, compressionThreshold >= 0);
    }

	auto response = std::static_pointer_cast<PacketLoginSuccess>(packet);

	if (response->Username != username) {
		throw std::logic_error("Received username is not sended username: "+response->Username+" != "+username);
	}

    timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();

	state = Play;
}

NetworkClient::~NetworkClient() {
}

std::shared_ptr<Packet> NetworkClient::ReceivePacket() {
	if (toReceive.empty())
		return std::shared_ptr < Packet > (nullptr);
	auto ret = toReceive.front();
	toReceive.pop();
	return ret;
}

void NetworkClient::SendPacket(std::shared_ptr<Packet> packet) {
	toSend.push(packet);
}

void NetworkClient::UpdatePacket() {
    while (!toSend.empty()) {
        if (toSend.front() != nullptr)
            network.SendPacket(*toSend.front(), compressionThreshold);
        toSend.pop();
    }

    auto packet = network.ReceivePacket(state, compressionThreshold >= 0);
    if (packet.get() != nullptr) {
        if (packet->GetPacketId() != PacketNamePlayCB::KeepAliveCB) {
            toReceive.push(packet);
        }
        else {
            timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();
            auto packetKeepAlive = std::static_pointer_cast<PacketKeepAliveCB>(packet);
            auto packetKeepAliveSB = std::make_shared<PacketKeepAliveSB>(packetKeepAlive->KeepAliveId);
            network.SendPacket(*packetKeepAliveSB, compressionThreshold);
        }
    }
    using namespace std::chrono_literals;
    if (std::chrono::steady_clock::now() - timeOfLastKeepAlivePacket > 20s) {
        auto disconnectPacket = std::make_shared<PacketDisconnectPlay>();
        disconnectPacket->Reason = "Timeout: server not respond";
        toReceive.push(disconnectPacket);
    }
}