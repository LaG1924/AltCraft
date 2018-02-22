#include "GlobalState.hpp"

#include "NetworkClient.hpp"
#include "GameState.hpp"
#include "Render.hpp"
#include "DebugInfo.hpp"
#include "Event.hpp"

//Global game variables
std::unique_ptr<NetworkClient> nc;
std::unique_ptr<GameState> gs;
std::unique_ptr<Render> render;
bool isRunning;
bool isPhysRunning;
EventListener listener;
bool isMoving[5] = { 0,0,0,0,0 };
std::thread threadPhys;
State state;

void PhysExec();

void InitEvents() {
    /*
    * Network Events
    */

    listener.RegisterHandler("Exit", [](const Event&) {
        isRunning = false;
    });

    listener.RegisterHandler("ConnectToServer", [](const Event& eventData) {
		auto data = eventData.get <std::tuple<std::string, unsigned short, std::string>>(); //address,port,username
        if (std::get<0>(data) == "" || std::get<1>(data) == 0)
            LOG(FATAL) << "NOT VALID CONNECT-TO-SERVER EVENT";
        if (nc != nullptr) {
            LOG(ERROR) << "Already connected";
            return;
        }
        LOG(INFO) << "Connecting to server";
		PUSH_EVENT("Connecting",0);
        gs = std::make_unique<GameState>();
        isPhysRunning = true;
        threadPhys = std::thread(&PhysExec);
        try {
            nc = std::make_unique<NetworkClient>(std::get<0>(data),
                                                 std::get<1>(data),
                                                 std::get<2>(data));
        } catch (std::exception &e) {
            LOG(WARNING) << "Connection failed";
			PUSH_EVENT("ConnectionFailed", std::string(e.what()));
            isPhysRunning = false;
            threadPhys.join();
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
        isPhysRunning = false;
        threadPhys.join();
        gs.reset();
    });

    listener.RegisterHandler("SendChatMessage", [](const Event& eventData) {
		auto message = eventData.get<std::string>();
        auto packet = std::static_pointer_cast<Packet>(std::make_shared<PacketChatMessageSB>(message));
        PUSH_EVENT("SendPacket",packet);
    });
}

void PhysExec() {
    EventListener listener;

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

    LoopExecutionTimeController timer(std::chrono::milliseconds(8));

    while (isPhysRunning) {
        DebugInfo::gameThreadTime = timer.GetRealDeltaS() * 1000'00.0f;

        if (state == State::Playing) {
            if (isMoving[GameState::FORWARD])
                gs->HandleMovement(GameState::FORWARD, timer.GetRealDeltaS());
            if (isMoving[GameState::BACKWARD])
                gs->HandleMovement(GameState::BACKWARD, timer.GetRealDeltaS());
            if (isMoving[GameState::LEFT])
                gs->HandleMovement(GameState::LEFT, timer.GetRealDeltaS());
            if (isMoving[GameState::RIGHT])
                gs->HandleMovement(GameState::RIGHT, timer.GetRealDeltaS());
            if (isMoving[GameState::JUMP])
                gs->HandleMovement(GameState::JUMP, timer.GetRealDeltaS());
        }

        gs->Update(timer.GetRealDeltaS());

		listener.HandleAllEvents();

        timer.Update();
    }
}

void GlobalState::Exec() {
    render = std::make_unique<Render>(900, 480, "AltCraft");
    isRunning = true;
    InitEvents();
    GlobalState::SetState(State::MainMenu);
    while (isRunning) {
        render->Update();
        listener.HandleAllEvents();
    }
    PUSH_EVENT("Exit", 0);
    isRunning = false;
    render.reset();
}

GameState *GlobalState::GetGameState() {
    return gs.get();
}

Render *GlobalState::GetRender() {
    return render.get();
}

State GlobalState::GetState() {
    return state;
}

void GlobalState::SetState(const State &newState) {
	if (newState != state)
		PUSH_EVENT("StateUpdated", 0);
    state = newState;
}