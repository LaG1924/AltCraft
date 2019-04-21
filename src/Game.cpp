#include "Game.hpp"

#include <memory>

#include "Render.hpp"
#include "GameState.hpp"
#include "NetworkClient.hpp"

bool isRunning = true;
bool isMoving[5] = { 0,0,0,0,0 };
State state;
std::unique_ptr<NetworkClient> nc;
std::unique_ptr<GameState> gs;
std::unique_ptr<Render> render;
EventListener listener;

void InitEvents() {
	/*
	* Network Events
	*/

	listener.RegisterHandler("ConnectToServer", [](const Event & eventData) {
		auto data = eventData.get <std::tuple<std::string, unsigned short, std::string>>(); //address,port,username
		if (std::get<0>(data) == "" || std::get<1>(data) == 0)
			LOG(FATAL) << "NOT VALID CONNECT-TO-SERVER EVENT";
		if (nc != nullptr) {
			LOG(ERROR) << "Already connected";
			return;
		}
		LOG(INFO) << "Connecting to server at address " + std::get<0>(data) + ":" + std::to_string(std::get<1>(data)) + " as " + std::get<2>(data);
		PUSH_EVENT("Connecting",0);
		gs = std::make_unique<GameState>();
		try {
			nc = std::make_unique<NetworkClient>(std::get<0>(data),
				std::get<1>(data),
				std::get<2>(data));
		} catch (std::exception &e) {
			LOG(WARNING) << "Connection failed";
			PUSH_EVENT("ConnectionFailed", std::string(e.what()));
			gs.reset();
			return;
		}
		LOG(INFO) << "Connected to server";
		PUSH_EVENT("ConnectionSuccessfull", 0);
		});

	listener.RegisterHandler("Disconnect", [](const Event& eventData) {
		auto data = eventData.get<std::string>();
		PUSH_EVENT("Disconnected", data);
		LOG(INFO) << "Disconnected: " << data;
		nc.reset();
		});

	listener.RegisterHandler("NetworkClientException", [](const Event& eventData) {
		auto data = eventData.get < std::string>();
		PUSH_EVENT("Disconnect", data);
		});

	/*
	* GameState Events
	*/

	listener.RegisterHandler("Exit", [](const Event&) {
		isRunning = false;
		});

	listener.RegisterHandler("Disconnected", [](const Event&) {
		if (!gs)
			return;
		gs.reset();
		});

	listener.RegisterHandler("SendChatMessage", [](const Event& eventData) {
		auto message = eventData.get<std::string>();
		auto packet = std::static_pointer_cast<Packet>(std::make_shared<PacketChatMessageSB>(message));
		PUSH_EVENT("SendPacket",packet);
		});

	/*
	* Phys Events
	*/

	listener.RegisterHandler("KeyPressed", [](const Event& eventData) {
		if (!gs)
			return;
		switch (eventData.get<SDL_Scancode>()) {
		case SDL_SCANCODE_W:
			isMoving[GameState::FORWARD] = true;
			break;
		case SDL_SCANCODE_A:
			isMoving[GameState::LEFT] = true;
			break;
		case SDL_SCANCODE_S:
			isMoving[GameState::BACKWARD] = true;
			break;
		case SDL_SCANCODE_D:
			isMoving[GameState::RIGHT] = true;
			break;
		case SDL_SCANCODE_SPACE:
			isMoving[GameState::JUMP] = true;
			break;
		default:
			break;
		}
		});

	listener.RegisterHandler("KeyReleased", [](const Event& eventData) {
		if (!gs)
			return;
		switch (eventData.get<SDL_Scancode>()) {
		case SDL_SCANCODE_W:
			isMoving[GameState::FORWARD] = false;
			break;
		case SDL_SCANCODE_A:
			isMoving[GameState::LEFT] = false;
			break;
		case SDL_SCANCODE_S:
			isMoving[GameState::BACKWARD] = false;
			break;
		case SDL_SCANCODE_D:
			isMoving[GameState::RIGHT] = false;
			break;
		case SDL_SCANCODE_SPACE:
			isMoving[GameState::JUMP] = false;
			break;
		default:
			break;
		}
		});

	listener.RegisterHandler("MouseMove", [](const Event& eventData) {
		if (!gs)
			return;
		auto data = eventData.get<std::tuple<double,double>>();
		gs->HandleRotation(std::get<0>(data),std::get<1>(data));
		});

	listener.RegisterHandler("ReceivedPacket", [](const Event& eventData) {
		std::shared_ptr<Packet> packet = eventData.get<std::shared_ptr<Packet>>();
		gs->UpdatePacket(packet);
		});

	listener.RegisterHandler("LmbPressed",[](const Event& eventData) {
		gs->StartDigging();
		});

	listener.RegisterHandler("LmbReleased",[](const Event& eventData) {
		gs->CancelDigging();
		});

	listener.RegisterHandler("RmbPressed", [](const Event& eventData) {
		gs->PlaceBlock();
		});

	listener.RegisterHandler("SelectedBlockChanged", [](const Event& eventData) {
		//TODO:
		//gs->CancelDigging();
		});
}

void RunGame() {
	InitEvents();

	render = std::make_unique<Render>(900, 480, "AltCraft");

	SetState(State::MainMenu);
	LoopExecutionTimeController time(std::chrono::milliseconds(16));

	while (isRunning) {
		listener.HandleAllEvents();
		if (gs)
			gs->Update(time.GetDeltaS());
		render->Update();
		time.Update();
	}

	render.reset();
}

State GetState() {
	return state;
}

void SetState(State newState) {
	if (newState != state)
		PUSH_EVENT("StateUpdated", 0);
	state = newState;
}

GameState *GetGameState() {
	return gs.get();
}

Render *GetRender() {
	return render.get();
}

NetworkClient *GetNetworkClient() {
	return nc.get();
}
