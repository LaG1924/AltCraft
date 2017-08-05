#include "ThreadNetwork.hpp"

ThreadNetwork::ThreadNetwork() {

}

ThreadNetwork::~ThreadNetwork() {
	delete nc;
}

void ThreadNetwork::Execute() {
	state = GlobalState::InitialLoading;
	EventListener listener;

	listener.RegisterHandler(EventType::GlobalAppState, [this](EventData eventData) {
		auto data = std::get<GlobalAppStateData>(eventData);
		state = data.state;
	});

	listener.RegisterHandler(EventType::ConnectToServer, [this](EventData eventData) {
		auto data = std::get<ConnectToServerData>(eventData);
		if (data.address == "" || data.port == 0)
			LOG(FATAL) << "NOT VALID CONNECT-TO-SERVER EVENT";
		if (nc != nullptr) {
			LOG(ERROR) << "Already connected";
			return;
		}
		SetGlobalState(GlobalState::Connecting);
		LOG(INFO) << "Connecting to server";
		try {
			nc = new NetworkClient(data.address, data.port, "HelloOne");
		} catch (std::exception &e) {
			LOG(WARNING) << "Connection failed";
			EventAgregator::PushEvent(EventType::ConnectionFailed, ConnectionFailedData{e.what()});
            return;
		}
		LOG(INFO) << "Connected to server";
		EventAgregator::PushEvent(EventType::ConnectionSuccessfull, ConnectionSuccessfullData{nc});
	});

	listener.RegisterHandler(EventType::RequestNetworkClient, [this](EventData eventData) {
		EventAgregator::PushEvent(EventType::RegisterNetworkClient, RegisterNetworkClientData{nc});
	});

	while (state != GlobalState::Exiting) {
		listener.HandleEvent();
	}
}