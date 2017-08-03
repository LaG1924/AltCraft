#include "ThreadRender.hpp"

ThreadRender::ThreadRender() {
	render = new Render(900, 480, "AltCraft");
}

ThreadRender::~ThreadRender() {
	delete render;
}

void ThreadRender::Execute() {
	render->ExecuteRenderLoop();
}
