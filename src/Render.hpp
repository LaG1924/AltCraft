#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <SDL.h>

#include "Utility.hpp"
#include "Renderer.hpp"
#include "Event.hpp"

class RendererWorld;

class Render {
    SDL_Window *window;
    SDL_GLContext glContext;

    bool renderGui = false;
	bool isMouseCaptured = false;
    int prevMouseX, prevMouseY;
	float mouseXDelta, mouseYDelta;
    std::unique_ptr<RendererWorld> world; 
    bool renderWorld = false;
    RenderState renderState;
    LoopExecutionTimeController timer;
    std::map<SDL_Scancode, bool> isKeyPressed;
    bool HasFocus=true;
    float sensetivity = 0.1f;
    bool isWireframe = false;
    std::vector<std::string> chatMessages;
	EventListener listener;
    std::string stateString;
	char fieldUsername[512];
	char fieldServerAddr[512];
	int fieldServerPort;
	float fieldDistance;
	float fieldSensetivity;
	float fieldTargetFps;
	bool fieldWireframe;
	bool fieldVsync;

	void SetMouseCapture(bool IsCaptured);

	void HandleMouseCapture();

	void HandleEvents();

	void InitSdl(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle);

	void InitGlew();

	void RenderFrame();

	void PrepareToRendering();

    void UpdateKeyboard();

    void RenderGui();

	void InitEvents();

public:
	Render(unsigned int windowWidth, unsigned int windowHeight, std::string windowTitle);
	~Render();

	void Update();
};