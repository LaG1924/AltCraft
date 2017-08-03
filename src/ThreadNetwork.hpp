#pragma once

#include "Thread.hpp"
#include "NetworkClient.hpp"
#include "Event.hpp"

class ThreadNetwork : Thread {
	NetworkClient *nc = nullptr;
	GlobalState state;
public:
	ThreadNetwork();
	~ThreadNetwork();
	void Execute() override;
};