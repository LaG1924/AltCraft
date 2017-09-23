#include "ThreadGame.hpp"

#include "DebugInfo.hpp"

ThreadGame::ThreadGame() {

}

ThreadGame::~ThreadGame() {

}

void ThreadGame::Execute() {

	EventListener listener;

    listener.RegisterHandler(EventType::Exit, [this] (EventData eventData) {
        isRunning = false;
    });

	listener.RegisterHandler(EventType::ConnectionSuccessfull, [this](EventData eventData) {
		auto data = std::get<ConnectionSuccessfullData>(eventData);
        gs = std::make_shared<GameState>(data.ptr);
        gs->gs = gs;
	});

    listener.RegisterHandler(EventType::Disconnected, [this](EventData eventData) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (!gs)
            return;
        gs->gs.reset();
        gs.reset();
    });

    listener.RegisterHandler(EventType::KeyPressed, [this](EventData eventData) {
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

    listener.RegisterHandler(EventType::KeyReleased, [this](EventData eventData) {
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

    listener.RegisterHandler(EventType::MouseMoved, [this](EventData eventData) {
        if (!gs)
            return;
        auto data = std::get<MouseMovedData>(eventData);
        gs->HandleRotation(data.x, data.y);
    });

	LoopExecutionTimeController timer(std::chrono::milliseconds(int(1.0f / 120.0f * 1000.0f)));

	while (isRunning) {
        DebugInfo::gameThreadTime = timer.GetRealDeltaS() * 1000'00.0f;
        if (gs != nullptr)
            gs->Update(timer.GetRealDeltaS());
        listener.HandleEvent();
        if (gs != nullptr) {         
            gs->UpdatePacket();
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
		timer.Update();        
	}
    gs.reset();
}
