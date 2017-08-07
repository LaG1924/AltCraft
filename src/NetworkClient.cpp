#include "NetworkClient.hpp"

NetworkClient::NetworkClient(std::string address, unsigned short port, std::string username)
		: network(address, port) {
	state = Handshaking;

	PacketHandshake handshake;
	handshake.protocolVersion = 335;
	handshake.serverAddress = address;
	handshake.serverPort = port;
	handshake.nextState = 2;
	network.SendPacket(handshake);
	state = Login;

	PacketLoginStart loginStart;
	loginStart.Username = "HelloOne";
	network.SendPacket(loginStart);

	auto response = std::static_pointer_cast<PacketLoginSuccess>(network.ReceivePacket(Login));
	if (response->Username != username) {
		throw std::logic_error("Received username is not sended username");
	}

	state = Play;

	isActive = true;
	networkThread = std::thread(&NetworkClient::NetworkLoop, this);
}

NetworkClient::~NetworkClient() {
	isActive = false;
	networkThread.join();
}

std::shared_ptr<Packet> NetworkClient::ReceivePacket() {
	if (toReceive.empty())
		return std::shared_ptr < Packet > (nullptr);
	toReceiveMutex.lock();
	auto ret = toReceive.front();
	toReceive.pop();
	toReceiveMutex.unlock();
	return ret;
}

void NetworkClient::SendPacket(std::shared_ptr<Packet> packet) {
	toSendMutex.lock();
	toSend.push(packet);
	toSendMutex.unlock();
}

void NetworkClient::NetworkLoop() {
	auto timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();
	el::Helpers::setThreadName("Network");
    bool validEnded = true;

	try {
		while (isActive) {
			toSendMutex.lock();
			while (!toSend.empty()) {
				if (toSend.front() != nullptr)
					network.SendPacket(*toSend.front());
				toSend.pop();
			}
			toSendMutex.unlock();
			auto packet = network.ReceivePacket(state);
			if (packet.get() != nullptr) {
				if (packet->GetPacketId() != PacketNamePlayCB::KeepAliveCB) {
					toReceiveMutex.lock();
					toReceive.push(packet);
					toReceiveMutex.unlock();
				} else {
					timeOfLastKeepAlivePacket = std::chrono::steady_clock::now();
					auto packetKeepAlive = std::static_pointer_cast<PacketKeepAliveCB>(packet);
					auto packetKeepAliveSB = std::make_shared<PacketKeepAliveSB>(packetKeepAlive->KeepAliveId);
					network.SendPacket(*packetKeepAliveSB);
				}
			}
			using namespace std::chrono_literals;
			if (std::chrono::steady_clock::now() - timeOfLastKeepAlivePacket > 20s) {
				auto disconnectPacket = std::make_shared<PacketDisconnectPlay>();
				disconnectPacket->Reason = "Timeout: server not respond";
				toReceiveMutex.lock();
				toReceive.push(disconnectPacket);
				toReceiveMutex.unlock();
				break;
			}
		}
	} catch (std::exception &e) {
        EventAgregator::PushEvent(EventType::NetworkClientException, NetworkClientExceptionData{ e.what() });		
        validEnded = false;
	}
}