#include "ThreadNetwork.hpp"

ThreadNetwork::ThreadNetwork() {

}

ThreadNetwork::~ThreadNetwork() {	
}

void ThreadNetwork::Execute() {
	EventListener listener;

    listener.RegisterHandler(EventType::Exit, [this] (EventData eventData) {
        isRunning = false;
    });

	listener.RegisterHandler(EventType::ConnectToServer, [this](EventData eventData) {
		auto data = std::get<ConnectToServerData>(eventData);
		if (data.address == "" || data.port == 0)
			LOG(FATAL) << "NOT VALID CONNECT-TO-SERVER EVENT";
		if (nc != nullptr) {
			LOG(ERROR) << "Already connected";
			return;
		}
		LOG(INFO) << "Connecting to server";
        EventAgregator::PushEvent(EventType::Connecting, ConnectingData{});
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

    listener.RegisterHandler(EventType::Disconnect, [this](EventData eventData) {
        auto data = std::get<DisconnectData>(eventData);
        EventAgregator::PushEvent(EventType::Disconnected, DisconnectedData{ data.reason });
        LOG(INFO) << "Disconnected: " << data.reason;
        delete nc;
        nc = nullptr;
    });

    listener.RegisterHandler(EventType::NetworkClientException, [this](EventData eventData) {
        auto data = std::get<NetworkClientExceptionData>(eventData);
        EventAgregator::PushEvent(EventType::Disconnect, DisconnectData{ data.what });
    });

    LoopExecutionTimeController timer(std::chrono::milliseconds(16));
	while (isRunning) {
		listener.HandleEvent();
        
        timer.Update();
	}
    delete nc;
}