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
class Framebuffer;
class RmlRenderInterface;
class RmlSystemInterface;
class RmlFileInterface;
namespace Rml
{
	class Context;
}

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
    std::map<SDL_Scancode, bool> isKeyPressed;
    bool HasFocus=true;
    float sensetivity = 0.1f;
    bool isWireframe = false;	
	std::unique_ptr<Framebuffer> framebuffer;
    std::vector<std::string> chatMessages;
	EventListener listener;
    std::string stateString;
	std::unique_ptr<RmlRenderInterface> rmlRender;
	std::unique_ptr<RmlSystemInterface> rmlSystem;
	std::unique_ptr<RmlFileInterface> rmlFile;
	Rml::Context* rmlContext;
	unsigned short sdlKeyMods = 0;

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

	void InitRml();

public:
	Render(unsigned int windowWidth, unsigned int windowHeight, std::string windowTitle);
	~Render();

	void Update();
};
