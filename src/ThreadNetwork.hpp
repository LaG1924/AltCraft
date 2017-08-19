#pragma once

#include "Thread.hpp"
#include "NetworkClient.hpp"
#include "Event.hpp"

class ThreadNetwork : Thread {
    std::shared_ptr<NetworkClient> nc;
    bool isRunning = true;
public:
	ThreadNetwork();
	~ThreadNetwork();
	void Execute() override;
};