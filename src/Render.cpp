#include "Render.hpp"

#include <easylogging++.h>
#include <optick.h>
#include <RmlUi/Core.h>
#include <RmlUi/Lua.h>

#include "Shader.hpp"
#include "AssetManager.hpp"
#include "Event.hpp"
#include "DebugInfo.hpp"
#include "Game.hpp"
#include "World.hpp"
#include "GameState.hpp"
#include "RendererWorld.hpp"
#include "Settings.hpp"
#include "Framebuffer.hpp"
#include "Plugin.hpp"
#include "Rml.hpp"

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
    {SDLK_TAB, Rml::Input::KI_TAB}
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

Render::Render(unsigned int windowWidth, unsigned int windowHeight,
               std::string windowTitle) {
    InitEvents();

	Settings::Load();

    InitSdl(windowWidth, windowHeight, windowTitle);
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

	//Read settings
	strcpy(fieldUsername, Settings::Read("username", "HelloOne").c_str());
	strcpy(fieldServerAddr, Settings::Read("serverAddr", "127.0.0.1").c_str());
	fieldDistance = Settings::ReadDouble("renderDistance", 2.0);
	fieldTargetFps = Settings::ReadDouble("targetFps", 60.0);
	fieldSensetivity = Settings::ReadDouble("mouseSensetivity", 0.1);
	fieldVsync = Settings::ReadBool("vsync", false);
	fieldWireframe = Settings::ReadBool("wireframe", false);
	fieldFlight = Settings::ReadBool("flight", false);
	fieldBrightness = Settings::ReadDouble("brightness", 0.2f);
	fieldResolutionScale = Settings::ReadDouble("resolutionScale", 1.0f);

	//Apply settings
	if (fieldSensetivity != sensetivity)
		sensetivity = fieldSensetivity;
	isWireframe = fieldWireframe;
	GetTime()->SetDelayLength(std::chrono::duration<double, std::milli>(1.0 / fieldTargetFps * 1000.0));
	if (fieldVsync) {
		GetTime()->SetDelayLength(std::chrono::milliseconds(0));
		SDL_GL_SetSwapInterval(1);
	}
	else
		SDL_GL_SetSwapInterval(0);
	framebuffer->Resize(renderState.WindowWidth * fieldResolutionScale, renderState.WindowHeight * fieldResolutionScale);

    LOG(INFO) << "Supported threads: " << std::thread::hardware_concurrency();
}

Render::~Render() {
	Settings::Write("username", fieldUsername);
	Settings::Write("serverAddr", fieldServerAddr);
	Settings::WriteDouble("renderDistance", fieldDistance);
	Settings::WriteDouble("targetFps", fieldTargetFps);
	Settings::WriteDouble("mouseSensetivity", fieldSensetivity);
	Settings::WriteBool("vsync", fieldVsync);
	Settings::WriteBool("wireframe", fieldWireframe);
	Settings::WriteBool("flight", fieldFlight);
	Settings::WriteDouble("brightness", fieldBrightness);
	Settings::WriteDouble("resolutionScale", fieldResolutionScale);
	Settings::Save();

    Rml::RemoveContext("default");
    rmlRender.reset();
    rmlSystem.reset();

	PluginSystem::Init();

	framebuffer.reset();
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

    window = SDL_CreateWindow(
        WinTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WinWidth, WinHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!window)
        throw std::runtime_error("Window creation failed: " + std::string(SDL_GetError()));

    glContext = SDL_GL_CreateContext(window);
    if (!glContext)
        throw std::runtime_error("OpenGl context creation failed: " + std::string(SDL_GetError()));

    SetMouseCapture(false);
    renderState.WindowWidth = WinWidth;
    renderState.WindowHeight = WinHeight;

    SDL_GL_SetSwapInterval(0);
}

void Render::InitGlew() {
    LOG(INFO) << "Initializing GLEW";
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    glCheckError();
    if (glewStatus != GLEW_OK) {
        LOG(FATAL) << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus);
    }
    int width, height;
    SDL_GL_GetDrawableSize(window, &width, &height);
    glViewport(0, 0, width, height);
	glClearColor(0.0f,0.0f,0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glCheckError();
    if (glActiveTexture == nullptr) {
        throw std::runtime_error("GLEW initialization failed with unknown reason");
    }
}

void Render::PrepareToRendering() {
    //TextureAtlas texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, AssetManager::GetTextureAtlasId());

	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	framebuffer = std::make_unique<Framebuffer>(width, height, true);
	Framebuffer::GetDefault().Activate();
	Framebuffer::GetDefault().Resize(width, height);
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
	framebuffer->Clear();
	Framebuffer::GetDefault().Clear();	

	if (renderWorld)
		framebuffer->Activate();
    if (isWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (renderWorld)
        world->Render(renderState);
    if (isWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	if (renderWorld)
		framebuffer->RenderTo(Framebuffer::GetDefault());

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
                        renderState.WindowWidth = width;
                        renderState.WindowHeight = height;
                        rmlRender->Update(width, height);
                        rmlContext->SetDimensions(Rml::Vector2i(width, height));
						framebuffer->Resize(width * fieldResolutionScale, height * fieldResolutionScale);
						Framebuffer::GetDefault().Resize(width, height);
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
                        else if (state == State::Chat) {
                            SetState(State::Playing);
                        }
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

    rmlContext->Render();
}

void Render::InitEvents() {
    listener.RegisterHandler("ConnectionSuccessfull", [this](const Event&) {
        stateString = "Logging in...";
    });

    listener.RegisterHandler("PlayerConnected", [this](const Event&) {
        stateString = "Loading terrain...";
        world = std::make_unique<RendererWorld>();
		world->MaxRenderingDistance = fieldDistance;
		PUSH_EVENT("UpdateSectionsRender", 0);		
    });

    listener.RegisterHandler("RemoveLoadingScreen", [this](const Event&) {
        stateString = "Playing";
        renderWorld = true;
        SetState(State::Playing);
		glClearColor(0, 0, 0, 1.0f);
		GetGameState()->GetPlayer()->isFlying = this->fieldFlight;
		PUSH_EVENT("SetMinLightLevel", fieldBrightness);
    });

    listener.RegisterHandler("ConnectionFailed", [this](const Event& eventData) {
        stateString = "Connection failed: " + eventData.get <std::string>();
        renderWorld = false;
        world.reset();
        SetState(State::MainMenu);
		glClearColor(0.8, 0.8, 0.8, 1.0f);
    });

    listener.RegisterHandler("Disconnected", [this](const Event& eventData) {
        stateString = "Disconnected: " + eventData.get<std::string>();
        renderWorld = false;
        world.reset();
        SetState(State::MainMenu);
		glClearColor(0.8, 0.8, 0.8, 1.0f);
    });

    listener.RegisterHandler("Connecting", [this](const Event&) {
        stateString = "Connecting to the server...";
        SetState(State::Loading);
    });

    listener.RegisterHandler("ChatMessageReceived", [this](const Event& eventData) {
        auto data = eventData.get<std::tuple<Chat, unsigned char>>();
        std::string msg = "(" + std::to_string((int)std::get<1>(data)) + ") " + (std::get<0>(data).ToPlainText());
        chatMessages.push_back(msg);
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
        }
    });
}

void Render::InitRml() {
    LOG(INFO) << "Initializing Rml";

    rmlSystem = std::make_unique<RmlSystemInterface>();
    Rml::SetSystemInterface(rmlSystem.get());

    rmlRender = std::make_unique<RmlRenderInterface>(renderState);
    Rml::SetRenderInterface(rmlRender.get());
    rmlRender->Update(renderState.WindowWidth, renderState.WindowHeight);

    rmlFile = std::make_unique<RmlFileInterface>();
    Rml::SetFileInterface(rmlFile.get());

    if (!Rml::Initialise())
        LOG(WARNING) << "Rml not initialized";

    Rml::Lua::Initialise(PluginSystem::GetLuaState());

    rmlContext = Rml::CreateContext("default", Rml::Vector2i(renderState.WindowWidth, renderState.WindowHeight));
}
