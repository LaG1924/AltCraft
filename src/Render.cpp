#include "Render.hpp"

#include "Utility.hpp"
#include "Shader.hpp"
#include "AssetManager.hpp"
#include "Event.hpp"
#include "DebugInfo.hpp"

#include <imgui.h>
#include "imgui_impl_sdl_gl3.h"

Render::Render(unsigned int windowWidth, unsigned int windowHeight, std::string windowTitle) : timer(std::chrono::milliseconds(0)) {
    InitSfml(windowWidth, windowHeight, windowTitle);
    glCheckError();
    InitGlew();
    glCheckError();
    PrepareToRendering();
    glCheckError();

    LOG(INFO) << "Supported threads: " << std::thread::hardware_concurrency();
}

Render::~Render() {
    ImGui_ImplSdlGL3_Shutdown();
    SDL_GL_DeleteContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Render::InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle) {
	LOG(INFO) << "Creating window: " << WinWidth << "x" << WinHeight << " \"" << WinTitle << "\"";
	
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        throw std::runtime_error("SDL initalization failed: " + std::string(SDL_GetError()));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    window = SDL_CreateWindow(WinTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WinWidth, WinHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!window)
        throw std::runtime_error("Window creation failed: " + std::string(SDL_GetError()));

    glContext = SDL_GL_CreateContext(window);
    if (!glContext)
        throw std::runtime_error("OpenGl context creation failed: " + std::string(SDL_GetError()));

	SetMouseCapture(false);    
    renderState.WindowWidth = WinWidth;
    renderState.WindowHeight = WinHeight;
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
	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);*/
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
    glBindTexture(GL_TEXTURE_2D, AssetManager::Instance().GetTextureAtlas());
    AssetManager::Instance().GetTextureAtlasIndexes();

    ImGui_ImplSdlGL3_Init(window);
}

void Render::UpdateKeyboard() {
    if (ImGui::GetIO().WantCaptureKeyboard)
        return;

    SDL_Scancode toUpdate[] = { SDL_SCANCODE_A,SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_D,SDL_SCANCODE_SPACE };
    const Uint8 *kbState = SDL_GetKeyboardState(0);
    for (auto key : toUpdate) {
        bool isPressed = kbState[key];
        if (!isKeyPressed[key] && isPressed) {
            EventAgregator::PushEvent(EventType::KeyPressed, KeyPressedData{ key });
        }
        if (isKeyPressed[key] && isPressed) {
            //KeyHeld
        }
        if (isKeyPressed[key] && !isPressed) {
            EventAgregator::PushEvent(EventType::KeyReleased, KeyReleasedData{ key });
        }
        isKeyPressed[key] = isPressed;
    }
}

void Render::RenderFrame() {	
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (isWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (renderWorld)
        world->Render(renderState);
    if (isWireframe)
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    if (world) {
        world->Update(timer.RemainTimeMs());
    }

    RenderGui();

    SDL_GL_SwapWindow(window);
}

void Render::HandleEvents() {
    SDL_PumpEvents();
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSdlGL3_ProcessEvent(&event);

        switch (event.type) {
        case SDL_QUIT:
            LOG(INFO) << "Received close event by window closing";
            isRunning = false;
            break;
        case SDL_WINDOWEVENT: {
            switch (event.window.event) {
            case SDL_WINDOWEVENT_RESIZED: {
                int width, height;
                SDL_GL_GetDrawableSize(window, &width, &height);
                glViewport(0, 0, width, height);
                renderState.WindowWidth = width;
                renderState.WindowHeight = height;
                break;
            }
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                HasFocus = true;
                break;
            case SDL_WINDOWEVENT_FOCUS_LOST:
                HasFocus = false;
                SetMouseCapture(false);
                if (state == GlobalState::Playing)
                    state = GlobalState::Paused;
                isDisplayInventory = false;
                break;
            }
            break;
        }
        case SDL_KEYDOWN:
            switch (event.key.keysym.scancode) {
            case SDL_SCANCODE_ESCAPE:
                if (state == GlobalState::Playing) {
                    state = GlobalState::Paused;
                    SetMouseCapture(false);
                    isDisplayInventory = false;
                }
                else if (state == GlobalState::Paused) {
                    state = GlobalState::Playing;
                    SetMouseCapture(true);
                }
                else if (state == GlobalState::MainMenu) {
                    LOG(INFO) << "Received close event by esc";
                    isRunning = false;
                }
                break;
            case SDL_SCANCODE_E:
                if (state != GlobalState::Playing)
                    return;
                isDisplayInventory = !isDisplayInventory;
                SetMouseCapture(!isDisplayInventory);
                break;            
            }
            break;        
        case SDL_MOUSEMOTION:
            if (isMouseCaptured) {
                double deltaX = event.motion.xrel;
                double deltaY = event.motion.yrel;                
                deltaX *= sensetivity;
                deltaY *= sensetivity * -1;
                EventAgregator::DirectEventCall(EventType::MouseMoved, MouseMovedData{ deltaX,deltaY });
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

void Render::ExecuteRenderLoop() {
	EventListener listener;

	listener.RegisterHandler(EventType::ConnectionSuccessfull, [this](EventData eventData) {
		auto data = std::get<ConnectionSuccessfullData>(eventData);
        stateString = "Logging in...";
	});

	listener.RegisterHandler(EventType::PlayerConnected, [this](EventData eventData) {
		auto data = std::get<PlayerConnectedData>(eventData);
        stateString = "Loading terrain...";
        world = std::make_unique<RendererWorld>(data.ptr);
	});

	listener.RegisterHandler(EventType::RemoveLoadingScreen, [this](EventData eventData) {
        stateString = "Playing";
        renderWorld = true;
        state = GlobalState::Playing;
        SetMouseCapture(true);
        glClearColor(0, 0, 0, 1.0f);
	});

    listener.RegisterHandler(EventType::ConnectionFailed, [this](EventData eventData) {
        stateString = "Connection failed: " + std::get<ConnectionFailedData>(eventData).reason;
        renderWorld = false;
        world.reset();
        state = GlobalState::MainMenu;
        glClearColor(0.8, 0.8, 0.8, 1.0f);
    });

    listener.RegisterHandler(EventType::Disconnected, [this](EventData eventData) {
        stateString = "Disconnected: " + std::get<DisconnectedData>(eventData).reason;
        renderWorld = false;
        world.reset();
        state = GlobalState::MainMenu;
        SetMouseCapture(false);
        glClearColor(0.8, 0.8, 0.8, 1.0f);
    });

    listener.RegisterHandler(EventType::Connecting, [this](EventData eventData) {
        stateString = "Connecting to the server...";
        state = GlobalState::Loading;
    });

    state = GlobalState::MainMenu;
	
	while (isRunning) {
		HandleEvents();
        if (HasFocus && state == GlobalState::Playing) UpdateKeyboard();
		if (isMouseCaptured) HandleMouseCapture();
		glCheckError();

		RenderFrame();
        while (listener.IsEventsQueueIsNotEmpty()) {
            listener.HandleEvent();
        }
		timer.Update();
	}
    EventAgregator::PushEvent(EventType::Exit, ExitData{});
}

void Render::RenderGui() {
    ImGui_ImplSdlGL3_NewFrame(window);

    if (isMouseCaptured) {
        auto& io = ImGui::GetIO();
        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    }
    const ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings;

    //ImGui::ShowTestWindow();

    ImGui::SetNextWindowPos(ImVec2(10, 10));
    ImGui::Begin("DebugInfo", 0, ImVec2(0, 0), 0.4f, windowFlags);
    ImGui::Text("Debug Info:");
    ImGui::Separator();
    ImGui::Text("State: %s", stateString.c_str());
    ImGui::Text("FPS: %.1f (%.3fms)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);    
    float gameTime = DebugInfo::gameThreadTime / 100.0f;
    ImGui::Text("TPS: %.1f (%.2fms)", 1000.0f/gameTime, gameTime);
    ImGui::Text("Sections loaded: %d", (int)DebugInfo::totalSections);
    ImGui::Text("SectionsRenderer: %d (%d)", (int)DebugInfo::renderSections, (int)DebugInfo::readyRenderer);
    if (world) {
        ImGui::Text("Player pos: %.1f  %.1f  %.1f", world->GameStatePtr()->g_PlayerX, world->GameStatePtr()->g_PlayerY, world->GameStatePtr()->g_PlayerZ);
        ImGui::Text("Player health: %.1f/%.1f", world->GameStatePtr()->g_PlayerHealth, 20.0f);
    }
    ImGui::End();


    switch (state) {
    case GlobalState::MainMenu: {
        ImGui::SetNextWindowPosCenter();
        ImGui::Begin("Menu",0, windowFlags);
        static char buff[512] = "127.0.0.1";
        static int port = 25565;
        if (ImGui::Button("Connect")) {
            EventAgregator::PushEvent(EventType::ConnectToServer, ConnectToServerData{ buff, (unsigned short)port });
        }
        ImGui::InputText("Address", buff, 512);
        ImGui::InputInt("Port", &port);        
        ImGui::Separator();
        if (ImGui::Button("Exit"))
            isRunning = false;
        ImGui::End();
        break;
    }
    case GlobalState::Loading:
        break;
    case GlobalState::Playing:
        if (isDisplayInventory) {
            auto renderSlot = [](const SlotData &slot, int i) -> bool {
                return ImGui::Button(((slot.BlockId == -1 ? "  ##" : 
                    AssetManager::Instance().GetAssetNameByBlockId(BlockId{ (unsigned short)slot.BlockId,0 }) +" x"+std::to_string(slot.ItemCount) + "##")
                    + std::to_string(i)).c_str());
            };
            ImGui::SetNextWindowPosCenter();
            ImGui::Begin("Inventory", 0, windowFlags);
            Window& inventory = world->GameStatePtr()->playerInventory;
            //Hand and drop slots
            if (renderSlot(inventory.handSlot, -1)) {

            }
            ImGui::SameLine();
            if (ImGui::Button("Drop")) {
                inventory.MakeClick(-1, true, true);
            }
            ImGui::SameLine();
            ImGui::Text("Hand slot and drop mode");
            ImGui::Separator();
            //Crafting
            if (renderSlot(inventory.slots[1], 1)) {
                inventory.MakeClick(1, true);
            }
            ImGui::SameLine();
            if (renderSlot(inventory.slots[2], 2)) {
                inventory.MakeClick(2, true);
            }
            //Crafting result
            ImGui::SameLine();
            ImGui::Text("Result");
            ImGui::SameLine();
            if (renderSlot(inventory.slots[0], 0)) {
                inventory.MakeClick(0, true);
            }
            //Crafting second line
            if (renderSlot(inventory.slots[3], 3)) {
                inventory.MakeClick(3, true);
            }
            ImGui::SameLine();
            if (renderSlot(inventory.slots[4], 4)) {
                inventory.MakeClick(4, true);
            }            
            ImGui::Separator();
            //Armor and offhand            
            for (int i = 5; i < 8+1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            if (renderSlot(inventory.slots[45], 45)) {
                inventory.MakeClick(45, true);
            }
            ImGui::SameLine();
            ImGui::Text("Armor and offhand");
            ImGui::Separator();
            for (int i = 36; i < 44+1; i++) {                
                if (renderSlot(inventory.slots[i], i)) {
                    inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::Text("Hotbar");
            ImGui::Separator();
            ImGui::Text("Main inventory");
            for (int i = 9; i < 17 + 1; i++) {  
                if (renderSlot(inventory.slots[i], i)) {                    
                    inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::Text("");
            for (int i = 18; i < 26 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::Text("");
            for (int i = 27; i < 35 + 1; i++) {
                if (renderSlot(inventory.slots[i], i)) {
                    inventory.MakeClick(i, true);
                }
                ImGui::SameLine();
            }
            ImGui::End();
        }
        break;
    case GlobalState::Paused: {
        ImGui::SetNextWindowPosCenter();
        ImGui::Begin("Pause Menu", 0, windowFlags);
        if (ImGui::Button("Continue")) {
            state = GlobalState::Playing;
            SetMouseCapture(true);
        }
        ImGui::Separator();
        static float distance = world->MaxRenderingDistance;
        ImGui::SliderFloat("Render distance", &distance, 1.0f, 16.0f);

        static float sense = sensetivity;
        ImGui::SliderFloat("Sensetivity", &sense, 0.01f, 1.0f);

        static bool wireframe = isWireframe;

        ImGui::Checkbox("Wireframe", &wireframe);

        if (ImGui::Button("Apply settings")) {
            if (distance != world->MaxRenderingDistance) {
                world->MaxRenderingDistance = distance;
                EventAgregator::PushEvent(EventType::UpdateSectionsRender, UpdateSectionsRenderData{});
            }

            if (sense != sensetivity)
                sensetivity = sense;

            isWireframe = wireframe;
        }
        ImGui::Separator();
        
        if (ImGui::Button("Disconnect")) {
            EventAgregator::PushEvent(EventType::Disconnect, DisconnectData{ "Disconnected by user" });
        }
        ImGui::End();
        break;
    }
    case GlobalState::InitialLoading:
        break;
    }

    ImGui::Render();
}