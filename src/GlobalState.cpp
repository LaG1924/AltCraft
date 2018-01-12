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
std::thread threadGs;
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
        try {
            nc = std::make_unique<NetworkClient>(std::get<0>(data), std::get<1>(data), std::get<2>(data));
        }
        catch (std::exception &e) {
            LOG(WARNING) << "Connection failed";
			PUSH_EVENT("ConnectionFailed", std::string(e.what()));
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

    listener.RegisterHandler("ConnectionSuccessfull", [](const Event&) {
        gs = std::make_unique<GameState>();
        isPhysRunning = true;
        threadPhys = std::thread(&PhysExec);
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
        nc->SendPacket(std::make_shared<PacketChatMessageSB>(message));
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

void GsExec() {
    el::Helpers::setThreadName("Game");
    LoopExecutionTimeController timer(std::chrono::milliseconds(16));

    while (isRunning) {
        try {
            while (nc && gs) {
                nc->UpdatePacket();
                
                gs->UpdatePacket(nc.get());
				listener.HandleAllEvents();
            }
        } catch (std::exception &e) {
			PUSH_EVENT("NetworkClientException", e.what());
        }

		listener.HandleAllEvents();

        timer.Update();
    }
    if (isPhysRunning) {
        isPhysRunning = false;
        threadPhys.join();
    }
    nc.reset();
    gs.reset();
}

void GlobalState::Exec() {
    render = std::make_unique<Render>(900, 480, "AltCraft");
    isRunning = true;
    InitEvents();
    threadGs = std::thread(&GsExec);
    render->ExecuteRenderLoop();
    isRunning = false;
    threadGs.join();
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