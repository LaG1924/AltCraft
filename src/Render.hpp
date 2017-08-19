#pragma once

#include <SFML/Window.hpp>

#include "Shader.hpp"
#include "RendererWorld.hpp"

class Render {
	sf::Window *window;
	bool isRunning=true;
	bool isMouseCaptured = false;
	float mouseXDelta, mouseYDelta;
    std::unique_ptr<RendererWorld> world; 
    bool renderWorld = false;
    RenderState renderState;
    LoopExecutionTimeController timer;

	void SetMouseCapture(bool IsCaptured);

	void HandleMouseCapture();

	void HandleEvents();

	void InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle);

	void InitGlew();

	void RenderFrame();

	void PrepareToRendering();
public:
	Render(unsigned int windowWidth, unsigned int windowHeight, std::string windowTitle);
	~Render();

	void ExecuteRenderLoop();
};