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
	state = Login;

	PacketLoginStart loginStart;
	loginStart.Username = username;

	size_t hslen = VarIntLen(handshake.GetPacketId()) + handshake.GetLen(),
			lslen = VarIntLen(loginStart.GetPacketId()) + loginStart.GetLen();
	StreamWOBuffer buffer(VarIntLen(hslen)+hslen + VarIntLen(lslen)+lslen);
	buffer.WriteVarInt(hslen);
	buffer.WriteVarInt(handshake.GetPacketId());
	handshake.ToStream(&buffer);

	buffer.WriteVarInt(lslen);
	buffer.WriteVarInt(loginStart.GetPacketId());
	loginStart.ToStream(&buffer);

	network->Connect(buffer.buffer, buffer.size);


    auto packet = network->ReceivePacket(Login);

	for (int i = 0; i < 10 && !packet; i++)
		packet = network->ReceivePacket(Login);
	if (!packet)
		throw std::runtime_error("Server not answered after LoginStart");

    if (packet->GetPacketId() == PacketNameLoginCB::SetCompression) {
        auto compPacket = std::static_pointer_cast<PacketSetCompression>(packet);
        LOG(INFO) << "Compression threshold: " << compPacket->Threshold;
        compressionThreshold = compPacket->Threshold;
        packet.reset();
		for (int i = 0; i < 10 && !packet; i++)
			packet = network->ReceivePacket(Login, compressionThreshold >= 0);
		if (!packet)
			throw std::runtime_error("Server not answered after SetCompression");
    }
	else if (packet->GetPacketId() == PacketNameLoginCB::Disconnect) {
		auto disconnectPacket = std::static_pointer_cast<PacketDisconnect>(packet);
		LOG(INFO) << "Server not allowed connection: " << disconnectPacket->Reason;
		throw std::runtime_error(disconnectPacket->Reason);
	}
	else if (packet->GetPacketId() != PacketNameLoginCB::LoginSuccess) {
		throw std::runtime_error("Unexpected packet type: " + std::to_string(packet->GetPacketId()));
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
	AC_THREAD_SET_NAME("Network");

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
