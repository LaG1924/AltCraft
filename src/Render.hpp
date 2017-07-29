#pragma once

#include <SFML/Window.hpp>

#include "Shader.hpp"

class Render {
	sf::Window *window;
	bool isRunning=true;
	bool isMouseCaptured = false;
	float mouseXDelta, mouseYDelta;
	Shader *shader;

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