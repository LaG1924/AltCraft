#pragma once

#include <SFML/Window.hpp>

#include "Shader.hpp"
#include "RendererWorld.hpp"
#include "RendererWidget.hpp"

class Render {
	sf::Window *window;
    bool isRunning = true;
	bool isMouseCaptured = false;
	float mouseXDelta, mouseYDelta;
    std::unique_ptr<RendererWorld> world; 
    bool renderWorld = false;
    RenderState renderState;
    LoopExecutionTimeController timer;
    std::map<sf::Keyboard::Key, bool> isKeyPressed;

	void SetMouseCapture(bool IsCaptured);

	void HandleMouseCapture();

	void HandleEvents();

	void InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle);

	void InitGlew();

	void RenderFrame();

	void PrepareToRendering();

    void UpdateKeyboard();
public:
	Render(unsigned int windowWidth, unsigned int windowHeight, std::string windowTitle);
	~Render();

	void ExecuteRenderLoop();
};