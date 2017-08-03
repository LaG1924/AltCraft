#pragma once

#include "Thread.hpp"
#include "GameState.hpp"
#include "Event.hpp"

class ThreadGame: Thread {
	GameState *gs = nullptr;
	GlobalState state = GlobalState::InitialLoading;
public:
	ThreadGame();
	~ThreadGame();
	void Execute() override;
};