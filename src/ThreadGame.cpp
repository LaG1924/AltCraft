#include "ThreadGame.hpp"


ThreadGame::ThreadGame() {

}

ThreadGame::~ThreadGame() {

}

void ThreadGame::Execute() {

	EventListener listener;

	listener.RegisterHandler(EventType::GlobalAppState, [this](EventData eventData) {
		auto data = std::get<GlobalAppStateData>(eventData);
		state = data.state;
	});

	listener.RegisterHandler(EventType::ConnectionSuccessfull, [this](EventData eventData) {
		auto data = std::get<ConnectionSuccessfullData>(eventData);
		gs = new GameState(data.ptr);
	});

	LoopExecutionTimeController timer(std::chrono::milliseconds(int(1.0f / 60.0f * 1000.0f)));

	while (state != GlobalState::Exiting) {
		listener.HandleEvent();
		if (gs != nullptr)
			gs->Update(timer.GetDeltaMs());
		timer.Update();
	}
	delete gs;
}
