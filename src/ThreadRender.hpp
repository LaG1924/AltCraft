#pragma once

#include "Thread.hpp"
#include "Render.hpp"

class ThreadRender : Thread {
	Render *render;
public:
	ThreadRender();
	~ThreadRender();
	void Execute() override;
};