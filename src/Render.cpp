#include "Render.hpp"

#include <imgui.h>
#include <easylogging++.h>

#include "imgui_impl_sdl_gl3.h"
#include "Shader.hpp"
#include "AssetManager.hpp"
#include "Event.hpp"
#include "DebugInfo.hpp"
#include "GlobalState.hpp"
#include "World.hpp"
#include "GameState.hpp"
#include "RendererWorld.hpp"
#include "Settings.hpp"
#include "Framebuffer.hpp"
#include "Plugin.hpp"

Render::Render(unsigned int windowWidth, unsigned int windowHeight,
               std::string windowTitle)
        : timer(std::chrono::milliseconds(16)) {
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
	timer.SetDelayLength(std::chrono::duration<double, std::milli>(1.0 / fieldTargetFps * 1000.0));
	if (fieldVsync) {
		timer.SetDelayLength(std::chrono::milliseconds(0));
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

	PluginSystem::Init();

	framebuffer.reset();
    ImGui_ImplSdlGL3_Shutdown();
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
	glClearColor(0.8,0.8,0.8, 1.0f);
    glEnable(GL_DEPTH_TEST);

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

    ImGui_ImplSdlGL3_Init(window);

	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	framebuffer = std::make_unique<Framebuffer>(width, height, true);
	Framebuffer::GetDefault().Activate();
	Framebuffer::GetDefault().Resize(width, height);
}

void Render::UpdateKeyboard() {
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

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
		world->Update(timer.RemainTimeMs());
	}

    SDL_GL_SwapWindow(window);
}

void Render::HandleEvents() {
    SDL_PumpEvents();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSdlGL3_ProcessEvent(&event);

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
						framebuffer->Resize(width * fieldResolutionScale, height * fieldResolutionScale);
						Framebuffer::GetDefault().Resize(width, height);
                        break;
                    }

                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                        HasFocus = true;
                        break;

                    case SDL_WINDOWEVENT_FOCUS_LOST: {
                        HasFocus = false;
                        auto state = GlobalState::GetState();
                        if (state == State::Inventory ||
                            state == State::Playing ||
                            state == State::Chat) {
                            GlobalState::SetState(State::Paused);
                        }
                        break;
                    }

                }
                break;
            }

            case SDL_KEYDOWN: {
                switch (event.key.keysym.scancode) {
                    case SDL_SCANCODE_ESCAPE: {
                        auto state = GlobalState::GetState();
                        if (state == State::Playing) {
                            GlobalState::SetState(State::Paused);
                        } else if (state == State::Paused ||
                                   state == State::Inventory ||
                                   state == State::Chat) {
                            GlobalState::SetState(State::Playing);
                        } else if (state == State::MainMenu) {
                            LOG(INFO) << "Received close event by esc";
                            PUSH_EVENT("Exit", 0);
                        }

                        break;
                    }

                    case SDL_SCANCODE_E: {
                        auto state = GlobalState::GetState();
                        if (state == State::Playing) {
                            GlobalState::SetState(State::Inventory);
                        } else if (state == State::Inventory) {
                            GlobalState::SetState(State::Playing);
                        }

                        break;
                    }

                    case SDL_SCANCODE_SLASH:
                    case SDL_SCANCODE_T: {
                        if (!ImGui::GetIO().WantCaptureKeyboard) {
                            auto state = GlobalState::GetState();
                            if (state == State::Playing) {
                                GlobalState::SetState(State::Chat);
                            } else if (state == State::Chat) {
                                GlobalState::SetState(State::Playing);
                            }
                        }

                        break;
                    }

                    default:
                        break;
                }

                break;
            }

            case SDL_MOUSEMOTION: {
                if (isMouseCaptured) {
                    double deltaX = event.motion.xrel;
                    double deltaY = event.motion.yrel;
                    deltaX *= sensetivity;
                    deltaY *= sensetivity * -1;
                    PUSH_EVENT("MouseMove", std::make_tuple(deltaX, deltaY));
                }

                break;
            }

            case SDL_MOUSEBUTTONDOWN: {
                if (isMouseCaptured && !ImGui::GetIO().WantCaptureMouse) {
                    if (event.button.button == SDL_BUTTON_LEFT)
                        PUSH_EVENT("LmbPressed", 0);
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                        PUSH_EVENT("RmbPressed", 0);
                }

                break;
            }

            case SDL_MOUSEBUTTONUP: {
                if (isMouseCaptured && !ImGui::GetIO().WantCaptureMouse) {
                    if (event.button.button == SDL_BUTTON_LEFT)
                        PUSH_EVENT("LmbReleased", 0);
                    else if (event.button.button == SDL_BUTTON_RIGHT)
                        PUSH_EVENT("RmbReleased", 0);
                }
                break;
            }

            default:
                break;
        }
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
	if (world)
		world->UpdateGameState(GlobalState::GetGameState());

    HandleEvents();
    if (HasFocus && GlobalState::GetState() == State::Playing) UpdateKeyboard();
    if (isMouseCaptured) HandleMouseCapture();
    glCheckError();

    RenderFrame();
    listener.HandleAllEvents();
    timer.Update();
}

void Render::RenderGui() {
    ImGui_ImplSdlGL3_NewFrame(window);

    if (isMouseCaptured) {
        auto& io = ImGui::GetIO();
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }

    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar |
                                         ImGuiWindowFlags_NoResize |
                                         ImGuiWindowFlags_NoMove |
                                         ImGuiWindowFlags_AlwaysAutoResize|
                                         ImGuiWindowFlags_NoSavedSettings;

    //ImGui::ShowTestWindow();

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("DebugInfo", 0, ImVec2(0, 0), 0.4f, windowFlags);
    ImGui::Text("Debug Info:");
    ImGui::Separator();
    ImGui::Text("State: %s", stateString.c_str());
    ImGui::Text("FPS: %.1f (%.3fms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
    float gameTime = DebugInfo::gameThreadTime / 100.0f;
    if (world) {
		Entity *playerPtr = world->GameStatePtr()->GetPlayer();
		SelectionStatus selectionStatus = world->GameStatePtr()->GetSelectionStatus();
		const World *worldPtr = &world->GameStatePtr()->GetWorld();

        ImGui::Text("TPS: %.1f (%.2fms)", 1000.0f / gameTime, gameTime);
        ImGui::Text("Sections loaded: %d", (int) DebugInfo::totalSections);
        ImGui::Text(
            "SectionsRenderer: %d (%d)",
            (int) DebugInfo::renderSections,(int) DebugInfo::readyRenderer);

        ImGui::Text(
            "Culled sections: %d",
            (int) DebugInfo::renderSections - world->culledSections);

		ImGui::Text(
			"Rendered faces: %d",
			(int)DebugInfo::renderFaces);

        ImGui::Text(
            "Player pos: %.1f  %.1f  %.1f  OnGround=%d",
			playerPtr->pos.x,
			playerPtr->pos.y,
			playerPtr->pos.z,
			playerPtr->onGround);

		ImGui::Text(
			"Player block pos: %d %d %d in %d %d %d",
			(int)(playerPtr->pos.x - std::floor(playerPtr->pos.x / 16.0) * 16),
			(int)(playerPtr->pos.y - std::floor(playerPtr->pos.y / 16.0) * 16),
			(int)(playerPtr->pos.z - std::floor(playerPtr->pos.z / 16.0) * 16),

			(int)std::floor(playerPtr->pos.x / 16.0),
			(int)std::floor(playerPtr->pos.y / 16.0),
			(int)std::floor(playerPtr->pos.z / 16.0));

        ImGui::Text(
            "Player vel: %.1f  %.1f  %.1f",
			playerPtr->vel.x,
			playerPtr->vel.y,
			playerPtr->vel.z);

        ImGui::Text(
            "Player health: %.1f/%.1f",
            world->GameStatePtr()->GetPlayerStatus().health, 20.0f);

        ImGui::Text(
            "Selected block: %d %d %d : %.1f",
            selectionStatus.selectedBlock.x,
			selectionStatus.selectedBlock.y,
			selectionStatus.selectedBlock.z,
			selectionStatus.distanceToSelectedBlock);

		ImGui::Text("Selected block light: %d  (%d)",
			worldPtr->GetBlockLight(selectionStatus.selectedBlock),
			worldPtr->GetBlockSkyLight(selectionStatus.selectedBlock));

		ImGui::Text("Selected block id: %d:%d  (%s)",
			worldPtr->GetBlockId(selectionStatus.selectedBlock).id,
			worldPtr->GetBlockId(selectionStatus.selectedBlock).state,
			AssetManager::GetAssetNameByBlockId(BlockId{ worldPtr->GetBlockId(selectionStatus.selectedBlock).id,0 }).c_str());

		ImGui::Text("Selected block variant: %s:%s",
			TransformBlockIdToBlockStateName(worldPtr->GetBlockId(selectionStatus.selectedBlock)).first.c_str(),
			TransformBlockIdToBlockStateName(worldPtr->GetBlockId(selectionStatus.selectedBlock)).second.c_str());
    }
    ImGui::End();


    switch (GlobalState::GetState()) {
        case State::MainMenu: {
            ImGui::SetNextWindowPosCenter();
            ImGui::Begin("Menu", 0, windowFlags);
            if (ImGui::Button("Connect")) {
				std::string addr(fieldServerAddr);
				size_t index = addr.find_last_of(':');
				unsigned short port;
				if (index == std::string::npos)
					port = 25565;
				else {
					try {
						port = std::stoi(addr.substr(index + 1));
					} catch (std::exception &e) {
						port = 25565;
					}
				}
				PUSH_EVENT("ConnectToServer", std::make_tuple(addr.substr(0, index), port, std::string(fieldUsername)));
            }
            ImGui::InputText("Username", fieldUsername, 512);
            ImGui::InputText("Address", fieldServerAddr, 512);
            ImGui::Separator();
            if (ImGui::Button("Exit"))
                PUSH_EVENT("Exit",0);
            ImGui::End();
            break;
        }

        case State::Loading:
            break;
            
        case State::Chat: {
            ImGui::SetNextWindowPosCenter();
            ImGui::Begin("Chat", 0, windowFlags);
            for (const auto& msg : chatMessages) {
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
                ImGui::TextWrapped("%s", msg.c_str());
                ImGui::PopStyleColor();
            }
            static char buff[256];
            ImGui::InputText("", buff, 256);
            ImGui::SameLine();
            if (ImGui::Button("Send")) {
                PUSH_EVENT("SendChatMessage", std::string(buff));
            }
            ImGui::End();
            break;
        }

        case State::Inventory: {
            auto renderSlot = [](const SlotDataType &slot, int i) -> bool {
                return ImGui::Button(((slot.BlockId == -1 ? "  ##" :
                    AssetManager::GetAssetNameByBlockId(BlockId{ (unsigned short)slot.BlockId,0 }) + " x" + std::to_string(slot.ItemCount) + "##")
                    + std::to_string(i)).c_str());
            };
            ImGui::SetNextWindowPosCenter();
            ImGui::Begin("Inventory", 0, windowFlags);
            const Window& inventory = world->GameStatePtr()->GetInventory();
            //Hand and drop slots
            if (renderSlot(inventory.handSlot, -1)) {

            }
            ImGui::SameLine();
            if (ImGui::Button("Drop")) {
                //inventory.MakeClick(-1, true, true);
            }
            ImGui::SameLine();
            ImGui::Text("Hand slot and drop mode");
            ImGui::Separator();
            //Crafting
            if (renderSlot(inventory.slots[1], 1)) {
                //inventory.MakeClick(1, true);
            }
            ImGui::SameLine();
            if (renderSlot(inventory.slots[2], 2)) {
                //inventory.MakeClick(2, true);
            }
            //Crafting result
            ImGui::SameLine();
            ImGui::Text("Result");
            ImGui::SameLine();
            if (renderSlot(inventory.slots[0], 0)) {
                //inventory.MakeClick(0, true);
            }
            //Crafting second line
            if (renderSlot(inventory.slots[3], 3)) {
                //inventory.MakeClick(3, true);
            }
            ImGui::SameLine();
            if (renderSlot(inventory.slots[4], 4)) {
                //inventory.MakeClick(4, true);
            }
            ImGui::Separator();
            //Armor and offhand
            for (int i = 5; i < 8 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    //inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            if (renderSlot(inventory.slots[45], 45)) {
                //inventory.MakeClick(45, true);
            }
            ImGui::SameLine();
            ImGui::Text("Armor and offhand");
            ImGui::Separator();
            for (int i = 36; i < 44 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    //inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::Text("Hotbar");
            ImGui::Separator();
            ImGui::Text("Main inventory");
            for (int i = 9; i < 17 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    //inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::Text("");
            for (int i = 18; i < 26 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    //inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::Text("");
            for (int i = 27; i < 35 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    //inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::End();

            break;
        }

        case State::Paused: {
            ImGui::SetNextWindowPosCenter();
            ImGui::Begin("Pause Menu", 0, windowFlags);
            if (ImGui::Button("Continue")) {
                GlobalState::SetState(State::Playing);
            }
            ImGui::Separator();

			ImGui::SliderFloat("Render distance", &fieldDistance, 1.0f, 16.0f);

			ImGui::SliderFloat("Brightness", &fieldBrightness, 0.0f, 1.0f);

			ImGui::SliderFloat("Sensetivity", &fieldSensetivity, 0.01f, 1.0f);

			ImGui::SliderFloat("Target FPS", &fieldTargetFps, 1.0f, 300.0f);

			ImGui::SliderFloat("Resolution scale", &fieldResolutionScale, 0.1f, 2.0f);

			ImGui::Checkbox("Wireframe", &fieldWireframe);

            ImGui::Checkbox("VSync", &fieldVsync);

			ImGui::Checkbox("Creative flight", &fieldFlight);
						
            if (ImGui::Button("Apply settings")) {
                if (fieldDistance != world->MaxRenderingDistance) {
                    world->MaxRenderingDistance = fieldDistance;
                    PUSH_EVENT("UpdateSectionsRender", 0);
                }

                if (fieldSensetivity != sensetivity)
                    sensetivity = fieldSensetivity;

				world->GameStatePtr()->GetPlayer()->isFlying = fieldFlight;

                isWireframe = fieldWireframe;
                timer.SetDelayLength(std::chrono::duration<double, std::milli>(1.0 / fieldTargetFps * 1000.0));
                if (fieldVsync) {
                    timer.SetDelayLength(std::chrono::milliseconds(0));
                    SDL_GL_SetSwapInterval(1);
                } else
                    SDL_GL_SetSwapInterval(0);

				PUSH_EVENT("SetMinLightLevel", fieldBrightness);

				int width, height;
				SDL_GL_GetDrawableSize(window, &width, &height);
				framebuffer->Resize(width * fieldResolutionScale, height * fieldResolutionScale);
            }
            ImGui::Separator();

            if (ImGui::Button("Disconnect")) {
                PUSH_EVENT("Disconnect", std::string("Disconnected by user"));
            }
            ImGui::End();
            break;
        }

        case State::InitialLoading:
            break;

        case State::Playing: {
            ImGui::SetNextWindowPosCenter();
            ImGui::Begin("",0,windowFlags);
            ImGui::End();
            break;
        }
    }

    ImGui::Render();
}

void Render::InitEvents() {
    listener.RegisterHandler("ConnectionSuccessfull", [this](const Event&) {
        stateString = "Logging in...";
    });

    listener.RegisterHandler("PlayerConnected", [this](const Event&) {
        stateString = "Loading terrain...";
        world = std::make_unique<RendererWorld>(GlobalState::GetGameState());
		world->MaxRenderingDistance = fieldDistance;
		PUSH_EVENT("UpdateSectionsRender", 0);		
    });

    listener.RegisterHandler("RemoveLoadingScreen", [this](const Event&) {
        stateString = "Playing";
        renderWorld = true;
        GlobalState::SetState(State::Playing);
		glClearColor(0, 0, 0, 1.0f);
		world->GameStatePtr()->GetPlayer()->isFlying = this->fieldFlight;
		PUSH_EVENT("SetMinLightLevel", fieldBrightness);
    });

    listener.RegisterHandler("ConnectionFailed", [this](const Event& eventData) {
        stateString = "Connection failed: " + eventData.get <std::string>();
        renderWorld = false;
        world.reset();
        GlobalState::SetState(State::MainMenu);
		glClearColor(0.8, 0.8, 0.8, 1.0f);
    });

    listener.RegisterHandler("Disconnected", [this](const Event& eventData) {
        stateString = "Disconnected: " + eventData.get<std::string>();
        renderWorld = false;
        world.reset();
        GlobalState::SetState(State::MainMenu);
		glClearColor(0.8, 0.8, 0.8, 1.0f);
    });

    listener.RegisterHandler("Connecting", [this](const Event&) {
        stateString = "Connecting to the server...";
        GlobalState::SetState(State::Loading);
    });

    listener.RegisterHandler("ChatMessageReceived", [this](const Event& eventData) {
        auto data = eventData.get<std::tuple<Chat, unsigned char>>();
        std::string msg = "(" + std::to_string((int)std::get<1>(data)) + ") " + std::get<0>(data).text;
        chatMessages.push_back(msg);
    });

    listener.RegisterHandler("StateUpdated", [this](const Event& eventData) {
        switch (GlobalState::GetState()) {
            case State::Playing:
                SetMouseCapture(true);
				PluginSystem::CallOnChangeState("Playing");
                break;
            case State::InitialLoading:
				PluginSystem::CallOnChangeState("InitialLoading");
				break;
            case State::MainMenu:
				PluginSystem::CallOnChangeState("MainMenu");
				break;
            case State::Loading:
				PluginSystem::CallOnChangeState("Loading");
				break;
            case State::Paused:
				PluginSystem::CallOnChangeState("Paused");
				break;
            case State::Inventory:
				PluginSystem::CallOnChangeState("Inventory");
				break;
            case State::Chat:
				PluginSystem::CallOnChangeState("Chat");
                SetMouseCapture(false);
                break;
        }
    });
}
