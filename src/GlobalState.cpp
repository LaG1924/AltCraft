#include "GlobalState.hpp"

#include "NetworkClient.hpp"
#include "GameState.hpp"
#include "Render.hpp"
#include "DebugInfo.hpp"


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

void PhysExec();

void InitEvents() {
    /*
    * Network Events
    */

    listener.RegisterHandler(EventType::Exit, [](EventData eventData) {
        isRunning = false;
    });

    listener.RegisterHandler(EventType::ConnectToServer, [](EventData eventData) {
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
            nc = std::make_unique<NetworkClient>(data.address, data.port, data.username);
        }
        catch (std::exception &e) {
            LOG(WARNING) << "Connection failed";
            EventAgregator::PushEvent(EventType::ConnectionFailed, ConnectionFailedData{ e.what() });
            return;
        }
        LOG(INFO) << "Connected to server";
        EventAgregator::PushEvent(EventType::ConnectionSuccessfull, ConnectionSuccessfullData{});
    });

    listener.RegisterHandler(EventType::Disconnect, [](EventData eventData) {
        auto data = std::get<DisconnectData>(eventData);
        EventAgregator::PushEvent(EventType::Disconnected, DisconnectedData{ data.reason });
        LOG(INFO) << "Disconnected: " << data.reason;
        nc.reset();
    });

    listener.RegisterHandler(EventType::NetworkClientException, [](EventData eventData) {
        auto data = std::get<NetworkClientExceptionData>(eventData);
        EventAgregator::PushEvent(EventType::Disconnect, DisconnectData{ data.what });
    });

    /*
    * GameState Events
    */

    listener.RegisterHandler(EventType::Exit, [](EventData eventData) {
        isRunning = false;
    });

    listener.RegisterHandler(EventType::ConnectionSuccessfull, [](EventData eventData) {
        auto data = std::get<ConnectionSuccessfullData>(eventData);
        gs = std::make_unique<GameState>();
        isPhysRunning = true;
        threadPhys = std::thread(&PhysExec);
    });

    listener.RegisterHandler(EventType::Disconnected, [](EventData eventData) {
        if (!gs)
            return;
        isPhysRunning = false;
        threadPhys.join();
        gs.reset();
    });

    listener.RegisterHandler(EventType::SendChatMessage, [](EventData eventData) {
        nc->SendPacket(std::make_shared<PacketChatMessageSB>(std::get<SendChatMessageData>(eventData).message));
    });
}

void PhysExec() {
    EventListener listener;

    listener.RegisterHandler(EventType::KeyPressed, [](EventData eventData) {
        if (!gs)
            return;
        switch (std::get<KeyPressedData>(eventData).key) {
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
        }
    });

    listener.RegisterHandler(EventType::KeyReleased, [](EventData eventData) {
        if (!gs)
            return;
        switch (std::get<KeyReleasedData>(eventData).key) {
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
        }
    });

    listener.RegisterHandler(EventType::MouseMoved, [](EventData eventData) {
        if (!gs)
            return;
        auto data = std::get<MouseMovedData>(eventData);
        gs->HandleRotation(data.x, data.y);
    });

    LoopExecutionTimeController timer(std::chrono::milliseconds(8));

    while (isPhysRunning) {
        DebugInfo::gameThreadTime = timer.GetRealDeltaS() * 1000'00.0f;

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

        gs->Update(timer.GetRealDeltaS());

        while (listener.IsEventsQueueIsNotEmpty())
            listener.HandleEvent();

        timer.Update();
    }
}

void GsExec() {
    LoopExecutionTimeController timer(std::chrono::milliseconds(16));

    while (isRunning) {
        try {
            while (nc && gs) {
                nc->UpdatePacket();
                
                gs->UpdatePacket(nc.get());
                while (listener.IsEventsQueueIsNotEmpty())
                    listener.HandleEvent();
            }
        } catch (std::exception &e) {
            EventAgregator::PushEvent(EventType::NetworkClientException, NetworkClientExceptionData{ e.what() });
        }

        while (listener.IsEventsQueueIsNotEmpty())
            listener.HandleEvent();

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