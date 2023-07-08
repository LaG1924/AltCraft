#pragma once

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <SDL.h>

#include "Utility.hpp"
#include "Event.hpp"
#include "Gal.hpp"

class Gbuffer;
class TextureFbCopy;
class RendererWorld;
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
    int prevMouseX=0, prevMouseY=0;
	float mouseXDelta=0.0f, mouseYDelta=0.0f;
    std::unique_ptr<RendererWorld> world; 
    bool renderWorld = false;
    size_t windowWidth, windowHeight;
    std::map<SDL_Scancode, bool> isKeyPressed;
    bool HasFocus=true;
    float sensetivity = 0.1f;
    bool isWireframe = false;
    std::unique_ptr<TextureFbCopy> resizeTextureCopy;
    std::unique_ptr<TextureFbCopy> fbTextureCopy;
    std::shared_ptr<Gal::Texture> fbTextureColor;
    std::shared_ptr<Gal::Texture> fbTextureDepthStencil;
    std::shared_ptr<Gal::Framebuffer> fbTarget;
    std::unique_ptr<Gbuffer> gbuffer;
    EventListener listener;
    std::string stateString;
    std::unique_ptr<RmlRenderInterface> rmlRender;
    std::unique_ptr<RmlSystemInterface> rmlSystem;
    std::unique_ptr<RmlFileInterface> rmlFile;
    Rml::Context* rmlContext;
    unsigned short sdlKeyMods = 0;
    bool hideRml = false;
    size_t renderBuff = 0;

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
	Render(unsigned int width, unsigned int height, std::string title);
	~Render();

	void Update();
};
