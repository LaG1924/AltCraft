#include "ThreadNetwork.hpp"

ThreadNetwork::ThreadNetwork() {

}

ThreadNetwork::~ThreadNetwork() {
	delete nc;
}

void ThreadNetwork::Execute() {
	bool isRunning;
	EventListener listener;

	listener.RegisterHandler(EventType::GlobalAppState, [this](EventData eventData) {
		auto data = std::get<GlobalAppStateData>(eventData);
		state = data.state;
	});

	listener.RegisterHandler(EventType::ConnectToServer, [this, &isRunning](EventData eventData) {
		auto data = std::get<ConnectToServerData>(eventData);
		if (data.address == "" || data.port == 0)
			LOG(FATAL) << "NOT VALID CONNECT-TO-SERVER EVENT";
		if (nc != nullptr) {
			LOG(ERROR) << "Already connected";
			return;
		}

		LOG(INFO) << "Connecting to server";
		try {
			nc = new NetworkClient(data.address, data.port, "HelloOne", isRunning);
		} catch (std::exception &e) {
			LOG(WARNING) << "CONNECTION FAIL";
			LOG(FATAL) << "Can't connect to server: " << e.what();
		}
		LOG(INFO) << "Connected to server";
		EventAgregator::PushEvent(EventType::ConnectionSuccessfull, ConnectionSuccessfullData{nc});
	});

	listener.RegisterHandler(EventType::RequestNetworkClient, [this](EventData eventData) {
		EventAgregator::PushEvent(EventType::RegisterNetworkClient, RegisterNetworkClientData{nc});
	});

	/*listener.RegisterHandler(EventType::SendPacket, [this](EventData eventData) {
		auto data = std::get<SendPacketData>(eventData);
		auto packet = data.packet;
		if (nc)
			nc->SendPacket(packet);
		else
			LOG(ERROR) << "Send packet, while not connected to server";
	});*/


	while (state != GlobalState::Exiting) {
		/*auto packet = nc ? nc->ReceivePacket() : nullptr;
		if (packet != nullptr)
			EventAgregator::PushEvent(EventType::ReceivePacket, ReceivePacketData{packet});*/
		listener.HandleEvent();
	}
}