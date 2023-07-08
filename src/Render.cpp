#include "Render.hpp"

#include <random>

#include <easylogging++.h>
#include <optick.h>
#include <RmlUi/Core.h>
#include <RmlUi/Lua.h>
#include <RmlUi/Debugger.h>

#include "AssetManager.hpp"
#include "Event.hpp"
#include "DebugInfo.hpp"
#include "Game.hpp"
#include "World.hpp"
#include "GameState.hpp"
#include "RendererWorld.hpp"
#include "Settings.hpp"
#include "Plugin.hpp"
#include "Rml.hpp"
#include "Gal.hpp"
#include "RenderConfigs.hpp"

const std::map<SDL_Keycode, Rml::Input::KeyIdentifier> keyMapping = {
    {SDLK_BACKSPACE, Rml::Input::KI_BACK},
    {SDLK_INSERT, Rml::Input::KI_INSERT},
    {SDLK_DELETE, Rml::Input::KI_DELETE},
    {SDLK_HOME, Rml::Input::KI_HOME},
    {SDLK_END, Rml::Input::KI_END},
    {SDLK_LEFT, Rml::Input::KI_LEFT},
    {SDLK_RIGHT, Rml::Input::KI_RIGHT},
    {SDLK_UP, Rml::Input::KI_UP},
    {SDLK_DOWN, Rml::Input::KI_DOWN},
    {SDLK_TAB, Rml::Input::KI_TAB},
    {SDLK_RETURN, Rml::Input::KI_RETURN}
};

inline int ConvertKeymodsSdlToRml(unsigned short keyMods) {
    int ret = 0;
    if (keyMods & KMOD_SHIFT)
        ret |= Rml::Input::KM_SHIFT;
    if (keyMods & KMOD_CTRL)
        ret |= Rml::Input::KM_CTRL;
    if (keyMods & KMOD_ALT)
        ret |= Rml::Input::KM_ALT;
    if (keyMods & KMOD_GUI)
        ret |= Rml::Input::KM_META;
    if (keyMods & KMOD_NUM)
        ret |= Rml::Input::KM_NUMLOCK;
    if (keyMods & KMOD_CAPS)
        ret |= Rml::Input::KM_CAPSLOCK;
    return ret;
}

Render::Render(unsigned int width, unsigned int height,
               std::string title) {
    InitEvents();

	Settings::Load();

    InitSdl(width, height, title);
    glCheckError();
    InitGlew();
    glCheckError();
	AssetManager::InitAssetManager();
	glCheckError();
    PrepareToRendering();
    glCheckError();
    InitRml();
    glCheckError();
    AssetManager::InitPostRml();
    glCheckError();


    LOG(INFO) << "Supported threads: " << std::thread::hardware_concurrency();
}

Render::~Render() {
    Rml::RemoveContext("default");
    rmlRender.reset();
    rmlSystem.reset();

    PluginSystem::Init();

    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Render::InitSdl(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle) {
    LOG(INFO) << "Creating window: " << WinWidth << "x" << WinHeight << " \"" << WinTitle << "\"";

    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("SDL initalization failed: " + std::string(SDL_GetError()));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
#ifndef NDEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif // !NDEBUG


    window = SDL_CreateWindow(
        WinTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WinWidth, WinHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
        throw std::runtime_error("Window creation failed: " + std::string(SDL_GetError()));

    glContext = SDL_GL_CreateContext(window);
    if (!glContext)
        throw std::runtime_error("OpenGl context creation failed: " + std::string(SDL_GetError()));

    SetMouseCapture(false);

    windowWidth = WinWidth;
    windowHeight = WinHeight;

    SDL_GL_SetSwapInterval(0);
}

void Render::InitGlew() {
    auto gal = Gal::GetImplementation();
    gal->Init();
    gal->GetGlobalShaderParameters()->Resize<GlobalShaderParameters>();

    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    gal->GetDefaultFramebuffer()->SetViewport(0, 0, width, height);
    gal->GetDefaultFramebuffer()->Clear();
}

void Render::PrepareToRendering() {
    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);

    float resolutionScale = Settings::ReadDouble("resolutionScale", 1.0f);
    size_t scaledW = width * resolutionScale, scaledH = height * resolutionScale;

    auto gal = Gal::GetImplementation();
    gal->GetDefaultFramebuffer()->SetViewport(0, 0, width, height);
    gal->GetGlobalShaderParameters()->Get<GlobalShaderParameters>()->gamma = Settings::ReadDouble("gamma", 2.2);

    gbuffer.reset();
    resizeTextureCopy.reset();
    fbTextureCopy.reset();
    fbTextureColor.reset();
    fbTextureDepthStencil.reset();
    fbTarget.reset();

    bool useDeffered = Settings::ReadBool("deffered", false);
    bool useResize = scaledW != width;

    if (useDeffered) {
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
        std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
        auto& ssaoKernels = gal->GetGlobalShaderParameters()->Get<GlobalShaderParameters>()->ssaoKernels;
        for (auto& vec : ssaoKernels) {
            vec.x = dis(rng);
            vec.y = dis(rng);
            vec.z = (dis(rng) + 1.0f) / 2.0f;
            vec.w = 0.0f;
            vec = glm::normalize(vec);
        }
        for (size_t i = 0; i < sizeof(ssaoKernels) / sizeof(*ssaoKernels); i++) {
            float scale = i / 64.0f;
            scale = glm::mix(0.1f, 1.0f, scale * scale);
            ssaoKernels[i] *= scale;
        }

        int ssaoSamples = Settings::ReadDouble("ssaoSamples", 0);
        float ssaoScale = Settings::ReadDouble("ssaoScale", 0.5f);
        size_t ssaoW = scaledW * ssaoScale, ssaoH = scaledH * ssaoScale;

        gbuffer = std::make_unique<Gbuffer>(scaledW, scaledH, scaledW, scaledH, ssaoSamples, ssaoW, ssaoH);
        gbuffer->SetRenderBuff(renderBuff);
        fbTarget = gbuffer->GetGeometryTarget();
        if (useResize) {
            auto fbTextureColorConf = gal->CreateTexture2DConfig(scaledW, scaledH, Gal::Format::R8G8B8);
            fbTextureColorConf->SetMinFilter(Gal::Filtering::Bilinear);
            fbTextureColorConf->SetMaxFilter(Gal::Filtering::Bilinear);
            fbTextureColor = gal->BuildTexture(fbTextureColorConf);

            resizeTextureCopy = std::make_unique<TextureFbCopy>(gbuffer->GetFinalTexture(), fbTextureColor);
            fbTextureCopy = std::make_unique<TextureFbCopy>(fbTextureColor, gal->GetDefaultFramebuffer());
        } else {
            fbTextureCopy = std::make_unique<TextureFbCopy>(gbuffer->GetFinalTexture(), gal->GetDefaultFramebuffer());
        }
    } else {
        if (useResize) {
            auto fbTextureColorConf = gal->CreateTexture2DConfig(scaledW, scaledH, Gal::Format::R8G8B8);
            fbTextureColorConf->SetMinFilter(Gal::Filtering::Bilinear);
            fbTextureColorConf->SetMaxFilter(Gal::Filtering::Bilinear);
            fbTextureColor = gal->BuildTexture(fbTextureColorConf);

            auto fbTextureDepthStencilConf = gal->CreateTexture2DConfig(scaledW, scaledH, Gal::Format::D24S8);
            fbTextureDepthStencilConf->SetMinFilter(Gal::Filtering::Bilinear);
            fbTextureDepthStencilConf->SetMaxFilter(Gal::Filtering::Bilinear);
            fbTextureDepthStencil = gal->BuildTexture(fbTextureDepthStencilConf);

            auto fbTargetConf = gal->CreateFramebufferConfig();
            fbTargetConf->SetTexture(0, fbTextureColor);
            fbTargetConf->SetDepthStencil(fbTextureDepthStencil);
            fbTarget = gal->BuildFramebuffer(fbTargetConf);
            fbTarget->SetViewport(0, 0, scaledW, scaledH);

            fbTextureCopy = std::make_unique<TextureFbCopy>(fbTextureColor, gal->GetDefaultFramebuffer());
        } else {
            fbTarget = gal->GetDefaultFramebuffer();
        }
    }

    if (world)
        world->PrepareRender(fbTarget, useDeffered);
}

void Render::UpdateKeyboard() {
    SDL_Scancode toUpdate[] = { SDL_SCANCODE_A,SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_D,SDL_SCANCODE_SPACE };
    const Uint8 *kbState = SDL_GetKeyboardState(0);
    for (auto key : toUpdate) {
        bool isPressed = kbState[key];
        if (!isKeyPressed[key] && isPressed) {
            PUSH_EVENT("KeyPressed", key);
        }
        if (isKeyPressed[key] && isPressed) {
            //KeyHeld
        }
        if (isKeyPressed[key] && !isPressed) {
            PUSH_EVENT("KeyReleased", key);
        }
        isKeyPressed[key] = isPressed;
    }
}

void Render::RenderFrame() {
    OPTICK_EVENT();

    Gal::GetImplementation()->GetDefaultFramebuffer()->Clear();
    if (gbuffer)
        gbuffer->Clear();
    if (fbTarget)
        fbTarget->Clear();
    if (resizeTextureCopy)
        resizeTextureCopy->Clear();
    if (fbTextureCopy)
        fbTextureCopy->Clear();

    if (isWireframe)
        Gal::GetImplementation()->SetWireframe(true);
    if (renderWorld) {
        world->Render(static_cast<float>(windowWidth) / static_cast<float>(windowHeight));
    }
    if (isWireframe)
        Gal::GetImplementation()->SetWireframe(false);

    if (gbuffer)
        gbuffer->Render();
    if (resizeTextureCopy)
        resizeTextureCopy->Copy();
    if (fbTextureCopy)
        fbTextureCopy->Copy();

    RenderGui();

    if (world) {
        world->Update(GetTime()->RemainTimeMs());
    }

    OPTICK_EVENT("VSYNC");
    SDL_GL_SwapWindow(window);
}

void Render::HandleEvents() {
    int rmlKeymods = ConvertKeymodsSdlToRml(sdlKeyMods);

    SDL_PumpEvents();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT: {
                LOG(INFO) << "Received close event by window closing";
                PUSH_EVENT("Exit",0);
                break;
            }

            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_RESIZED: {
                        int width, height;
                        SDL_GL_GetDrawableSize(window, &width, &height);
                        windowWidth = width;
                        windowHeight = height;
                        rmlRender->Update(width, height);
                        rmlContext->SetDimensions(Rml::Vector2i(width, height));
                        Gal::GetImplementation()->GetGlobalShaderParameters()->Get<GlobalShaderParameters>()->viewportSize = glm::uvec2(width, height);
                        PrepareToRendering();
                        break;
                    }

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        HasFocus = true;
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST: {
                        HasFocus = false;
                        State state = GetState();
                        if (state == State::Inventory ||
                            state == State::Playing ||
                            state == State::Chat) {
                            SetState(State::Paused);
                        }
                        break;
                    }

                }
                break;
            }

            case SDL_KEYDOWN: {
                sdlKeyMods = event.key.keysym.mod;
                rmlKeymods = ConvertKeymodsSdlToRml(sdlKeyMods);

                auto it = keyMapping.find(event.key.keysym.sym);
                Rml::Input::KeyIdentifier ki = it != keyMapping.end() ? it->second : Rml::Input::KeyIdentifier::KI_UNKNOWN;
                rmlContext->ProcessKeyDown(ki, rmlKeymods);

                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE: {
                        auto state = GetState();
                        if (state == State::Playing) {
                            SetState(State::Paused);
                        } else if (state == State::Paused ||
                                   state == State::Inventory ||
                                   state == State::Chat) {
                            SetState(State::Playing);
                        } else if (state == State::MainMenu) {
                            LOG(INFO) << "Received close event by esc";
                            PUSH_EVENT("Exit", 0);
                        }

                        break;
                    }

                    case SDL_SCANCODE_E: {
                        auto state = GetState();
                        if (state == State::Playing) {
                            SetState(State::Inventory);
                        } else if (state == State::Inventory) {
                            SetState(State::Playing);
                        }

                        break;
                    }

                    case SDL_SCANCODE_SLASH:
                    case SDL_SCANCODE_T: {
                        auto state = GetState();
                        if (state == State::Playing) {
                            SetState(State::Chat);
                        }
                        break;
                    }

                    case SDL_SCANCODE_F4:
                        hideRml = !hideRml;
                        break;

                    case SDL_SCANCODE_F8:
                        Rml::Debugger::SetVisible(!Rml::Debugger::IsVisible());
                        break;

                    case SDL_SCANCODE_F7: {
                        SetMouseCapture(!isMouseCaptured);
                        break;
                    }

                    case SDL_SCANCODE_F9: {
                        if (sdlKeyMods & KMOD_CTRL) {
                            renderBuff = 0;
                        } else if (sdlKeyMods & KMOD_SHIFT) {
                            renderBuff--;
                            if (renderBuff < 0)
                                renderBuff = 0;
                        } else {
                            renderBuff++;
                            if (renderBuff > gbuffer->GetMaxRenderBuffers())
                                renderBuff = 0;
                        }
                        if (gbuffer)
                            gbuffer->SetRenderBuff(renderBuff);
                        break;
                    }

                    default:
                        break;
                }

                break;
            }

            case SDL_KEYUP: {
                sdlKeyMods = event.key.keysym.mod;
                rmlKeymods = ConvertKeymodsSdlToRml(sdlKeyMods);

                auto it = keyMapping.find(event.key.keysym.sym);
                Rml::Input::KeyIdentifier ki = it != keyMapping.end() ? it->second : Rml::Input::KeyIdentifier::KI_UNKNOWN;
                rmlContext->ProcessKeyUp(ki, rmlKeymods);
                break;
            }

            case SDL_MOUSEMOTION: {
                if (isMouseCaptured) {
                    double deltaX = event.motion.xrel;
                    double deltaY = event.motion.yrel;
                    deltaX *= sensetivity;
                    deltaY *= sensetivity * -1;
                    PUSH_EVENT("MouseMove", std::make_tuple(deltaX, deltaY));
                } else {
                    int mouseX, mouseY;
                    SDL_GetMouseState(&mouseX, &mouseY);
                    rmlContext->ProcessMouseMove(mouseX, mouseY, rmlKeymods);
                }
                break;
            }

            case SDL_MOUSEBUTTONDOWN: {
                if (isMouseCaptured) {
                    if (event.button.button == SDL_BUTTON_LEFT)
                        PUSH_EVENT("LmbPressed", 0);
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                        PUSH_EVENT("RmbPressed", 0);
                } else {
                    if (event.button.button == SDL_BUTTON_MIDDLE)
                        event.button.button = SDL_BUTTON_RIGHT;
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                        event.button.button = SDL_BUTTON_MIDDLE;
                    rmlContext->ProcessMouseButtonDown(event.button.button - 1, rmlKeymods);
                }
                    
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                if (isMouseCaptured) {
                    if (event.button.button == SDL_BUTTON_LEFT)
                        PUSH_EVENT("LmbReleased", 0);
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                        PUSH_EVENT("RmbReleased", 0);
                } else {
                    if (event.button.button == SDL_BUTTON_MIDDLE)
                        event.button.button = SDL_BUTTON_RIGHT;
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                        event.button.button = SDL_BUTTON_MIDDLE;
                    rmlContext->ProcessMouseButtonUp(event.button.button - 1, rmlKeymods);
                }
                break;
            }

            case SDL_MOUSEWHEEL: {
                rmlContext->ProcessMouseWheel(-event.wheel.y, rmlKeymods);
                break;
            }

            case SDL_TEXTINPUT: {
                rmlContext->ProcessTextInput(Rml::String(event.text.text));
                break;
            }

            default:
                break;
        }
    }
    char* rawClipboard = SDL_GetClipboardText();
    std::string clipboard = rawClipboard;
    SDL_free(rawClipboard);

    if (clipboard != rmlSystem->clipboard) {
        rmlSystem->clipboard = clipboard;
    }

    rmlContext->Update();

    if (clipboard != rmlSystem->clipboard) {
        clipboard = rmlSystem->clipboard;
        SDL_SetClipboardText(clipboard.c_str());
    }
}

void Render::HandleMouseCapture() {
}

void Render::SetMouseCapture(bool IsCaptured) {
    if (IsCaptured == isMouseCaptured)
        return;
    isMouseCaptured = IsCaptured;

    if (isMouseCaptured) {
        SDL_GetGlobalMouseState(&prevMouseX, &prevMouseY);
    }

    SDL_CaptureMouse(IsCaptured ? SDL_TRUE : SDL_FALSE);
    SDL_SetRelativeMouseMode(IsCaptured ? SDL_TRUE : SDL_FALSE);

    if (!isMouseCaptured) {
        SDL_WarpMouseGlobal(prevMouseX, prevMouseY);
    }
}

void Render::Update() {
	OPTICK_EVENT();
    HandleEvents();
    if (HasFocus && GetState() == State::Playing) UpdateKeyboard();
    if (isMouseCaptured) HandleMouseCapture();
    glCheckError();

    RenderFrame();
    listener.HandleAllEvents();
}

void Render::RenderGui() {
	OPTICK_EVENT();

    if (!hideRml)
        rmlContext->Render();
}

void Render::InitEvents() {
    listener.RegisterHandler("ConnectionSuccessfull", [this](const Event&) {
        stateString = "Logging in...";
    });

    listener.RegisterHandler("PlayerConnected", [this](const Event&) {
        stateString = "Loading terrain...";
        world = std::make_unique<RendererWorld>(fbTarget, Settings::ReadBool("deffered", false), Settings::ReadBool("smoothlight", false));
        world->MaxRenderingDistance = Settings::ReadDouble("renderDistance", 2.0f);
		PUSH_EVENT("UpdateSectionsRender", 0);		
    });

    listener.RegisterHandler("RemoveLoadingScreen", [this](const Event&) {
        stateString = "Playing";
        renderWorld = true;
        SetState(State::Playing);
        GetGameState()->GetPlayer()->isFlying = Settings::ReadBool("flight", false);
    });

    listener.RegisterHandler("ConnectionFailed", [this](const Event& eventData) {
        stateString = "Connection failed: " + eventData.get <std::string>();
        renderWorld = false;
        world.reset();
        SetState(State::MainMenu);
        PluginSystem::CallOnDisconnected("Connection failed: " + eventData.get <std::string>());
    });

    listener.RegisterHandler("Disconnected", [this](const Event& eventData) {
        stateString = "Disconnected: " + eventData.get<std::string>();
        renderWorld = false;
        world.reset();
        SetState(State::MainMenu);
        PluginSystem::CallOnDisconnected("Disconnected: " + eventData.get<std::string>());
    });

    listener.RegisterHandler("Connecting", [this](const Event&) {
        stateString = "Connecting to the server...";
        SetState(State::Loading);
    });

    listener.RegisterHandler("StateUpdated", [this](const Event& eventData) {
        switch (GetState()) {
            case State::Playing:
                SetMouseCapture(true);
				PluginSystem::CallOnChangeState("Playing");
                break;
            case State::InitialLoading:
				PluginSystem::CallOnChangeState("InitialLoading");
				SetMouseCapture(false);
				break;
            case State::MainMenu:
				PluginSystem::CallOnChangeState("MainMenu");
				SetMouseCapture(false);
				break;
            case State::Loading:
				PluginSystem::CallOnChangeState("Loading");
				SetMouseCapture(false);
				break;
            case State::Paused:
				PluginSystem::CallOnChangeState("Paused");
				SetMouseCapture(false);
				break;
            case State::Inventory:
				PluginSystem::CallOnChangeState("Inventory");
				SetMouseCapture(false);
				break;
            case State::Chat:
				PluginSystem::CallOnChangeState("Chat");
                SetMouseCapture(false);
                break;
            case State::NeedRespawn:
                PluginSystem::CallOnChangeState("NeedRespawn");
                SetMouseCapture(false);
                break;
        }
    });

    listener.RegisterHandler("SettingsUpdate", [this](const Event& eventData) {
        if (world) {
            world->smoothLighting = Settings::ReadBool("smoothlight", false);
            float renderDistance = Settings::ReadDouble("renderDistance", 2.0f);
            if (renderDistance != world->MaxRenderingDistance) {
                world->MaxRenderingDistance = renderDistance;
                PUSH_EVENT("UpdateSectionsRender", 0);
            }
        }
        

        sensetivity = Settings::ReadDouble("mouseSensetivity", 0.1f);
        
        if (GetGameState()) {
            bool flight = Settings::ReadBool("flight", false);
            GetGameState()->GetPlayer()->isFlying = flight;
        }

        isWireframe = Settings::ReadBool("wireframe", false);

        float targetFps = Settings::ReadDouble("targetFps", 60.0f);
        GetTime()->SetDelayLength(std::chrono::duration<double, std::milli>(1.0 / targetFps * 1000.0));

        bool vsync = Settings::ReadBool("vsync", false);
        if (vsync) {
            GetTime()->SetDelayLength(std::chrono::milliseconds(0));
            SDL_GL_SetSwapInterval(1);
        }
        else
            SDL_GL_SetSwapInterval(0);


        Gal::GetImplementation()->GetGlobalShaderParameters()->Get<GlobalShaderParameters>()->gamma = Settings::ReadDouble("gamma", 2.2);

        PrepareToRendering();
    });
}

void Render::InitRml() {
    LOG(INFO) << "Initializing Rml";

    rmlSystem = std::make_unique<RmlSystemInterface>();
    Rml::SetSystemInterface(rmlSystem.get());

    rmlRender = std::make_unique<RmlRenderInterface>();
    Rml::SetRenderInterface(rmlRender.get());
    rmlRender->Update(windowWidth, windowHeight);
    Gal::GetImplementation()->GetGlobalShaderParameters()->Get<GlobalShaderParameters>()->viewportSize = glm::uvec2(windowWidth, windowHeight);

    rmlFile = std::make_unique<RmlFileInterface>();
    Rml::SetFileInterface(rmlFile.get());

    if (!Rml::Initialise())
        LOG(WARNING) << "Rml not initialized";

    Rml::Lua::Initialise(PluginSystem::GetLuaState());

    rmlContext = Rml::CreateContext("default", Rml::Vector2i(windowWidth, windowHeight));

    if (!Rml::Debugger::Initialise(rmlContext))
        LOG(WARNING) << "Rml debugger not initialized";
}
