#pragma once

#include "Thread.hpp"
#include "GameState.hpp"
#include "Event.hpp"

class ThreadGame: Thread {
	GameState *gs = nullptr;
    bool isRunning = true;
    bool isMoving[4] = { 0,0,0,0 };
public:
	ThreadGame();
	~ThreadGame();
	void Execute() override;
};