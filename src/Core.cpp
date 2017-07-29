#include "Core.hpp"
#include "NetworkClient.hpp"

Core::Core() {
	coreLoop = std::thread(&Core::ExecuteCoreLoop, this);
	render = new Render(900, 480, "AltCraft");
}

Core::~Core() {
	delete render;
	coreLoop.join();
}

void Core::ExecuteRenderLoop() {
	render->ExecuteRenderLoop();
}

void Core::ExecuteCoreLoop() {

	EventListener events;
	events.RegisterHandler(EventType::ConnectionSuccessfull, [](EventData eventData) {
		GlobalAppStateData data = {GlobalState::Loading};
		EventAgregator::PushEvent(EventType::GlobalAppState, EventData{data});
	});
	events.RegisterHandler(EventType::GlobalAppState, [this](EventData eventData) {
		auto data = std::get<GlobalAppStateData>(eventData);
		globalState = data.state;
	});

	while (globalState != GlobalState::Exiting) {
		if (events.IsEventsQueueIsNotEmpty()) {
			events.HandleEvent();
		}
	}
}

void Core::ExecuteNetworkLoop() {
	NetworkClient *nc;
	EventListener events;
	bool isRunning = true;
	events.RegisterHandler(EventType::ConnectToServer, [](EventData eventData) {
		auto data = std::get<ConnectToServerData>(eventData);
		try {
			nc = new NetworkClient(data.address, data.port, "HelloOne", isRunning);
		} catch (std::exception &e) {
			GlobalAppStateData data{GlobalState::Exiting};
			EventAgregator::PushEvent(EventType::GlobalAppState, data);
		}
	});
	events.RegisterHandler(EventType::Disconnect, [nc](EventData eventData) {
		delete nc;
		nc = nullptr;
	});

	while (globalState != GlobalState::Exiting) {

	}
	delete nc;
}
