#pragma once

#include "Thread.hpp"
#include "NetworkClient.hpp"
#include "Event.hpp"

class ThreadNetwork : Thread {
	NetworkClient *nc = nullptr;
    bool isRunning = true;
public:
	ThreadNetwork();
	~ThreadNetwork();
	void Execute() override;
};