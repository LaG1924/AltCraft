#include "NetworkClient.hpp"

#include <easylogging++.h>

#include "Network.hpp"
#include "Event.hpp"

NetworkClient::NetworkClient(std::string address, unsigned short port, std::string username) {
    network = std::make_unique<Network>(address, port);
	state = Handshaking;

	PacketHandshake handshake;
	handshake.protocolVersion = 340;
	handshake.serverAddress = address;
	handshake.serverPort = port;
	handshake.nextState = 2;
	network->SendPacket(handshake);
	state = Login;

	PacketLoginStart loginStart;
	loginStart.Username = username;
	network->SendPacket(loginStart);


    auto packet = network->ReceivePacket(Login);

    while (!packet)
        packet = network->ReceivePacket(Login);

    if (packet->GetPacketId() == PacketNameLoginCB::SetCompression) {
        auto compPacket = std::static_pointer_cast<PacketSetCompression>(packet);
        LOG(INFO) << "Compression threshold: " << compPacket->Threshold;
        compressionThreshold = compPacket->Threshold;
        packet.reset();
        while (!packet)
            packet = network->ReceivePacket(Login, compressionThreshold >= 0);
    }

	auto response = std::static_pointer_cast<PacketLoginSuccess>(packet);

	if (response->Username != username) {
		throw std::logic_error("Received username is not sended username: "+response->Username+" != "+username);
	}

    timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();

	state = Play;
    thread = std::thread(&NetworkClient::ExecNs,this);
}

NetworkClient::~NetworkClient() {
    isRunning = false;
    thread.join();
}

void NetworkClient::ExecNs() {
    EventListener listener;

    listener.RegisterHandler("SendPacket", [&](const Event& eventData) {
        std::shared_ptr<Packet> packet = eventData.get<std::shared_ptr<Packet>>();
        network->SendPacket(*packet,compressionThreshold);
    });

	try {
		while (isRunning) {
			listener.HandleAllEvents();

			std::shared_ptr<Packet> packet = network->ReceivePacket(state, compressionThreshold >= 0);
			if (packet != nullptr) {
				if (packet->GetPacketId() != PacketNamePlayCB::KeepAliveCB) {
					PUSH_EVENT("ReceivedPacket", packet);
				}
				else {
					timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();
					auto packetKeepAlive = std::static_pointer_cast<PacketKeepAliveCB>(packet);
					auto packetKeepAliveSB = std::make_shared<PacketKeepAliveSB>(packetKeepAlive->KeepAliveId);
					network->SendPacket(*packetKeepAliveSB, compressionThreshold);
				}
			}
			using namespace std::chrono_literals;
			if (std::chrono::steady_clock::now() - timeOfLastKeepAlivePacket > 20s) {
				packet = std::make_shared<PacketDisconnectPlay>();
				std::static_pointer_cast<PacketDisconnectPlay>(packet)->Reason = "Timeout: server not respond";
				PUSH_EVENT("ReceivedPacket", packet);
			}
		}
	} catch (std::exception &e) {
		PUSH_EVENT("NetworkClientException", std::string(e.what()));
	}
}