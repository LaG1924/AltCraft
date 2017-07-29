#pragma once

#include <thread>
#include <memory>

#include "Render.hpp"
#include "Event.hpp"

class Core {
	std::thread coreLoop;
	std::thread networkLoop;
	Render* render;

	GlobalState globalState = GlobalState::InitialLoading;

	void ExecuteCoreLoop();
	void ExecuteNetworkLoop();
public:
	Core();
	~Core();

	void ExecuteRenderLoop();
};