#include "Render.hpp"

#include "Utility.hpp"
#include "Shader.hpp"
#include "AssetManager.hpp"
#include "Event.hpp"

Render::Render(unsigned int windowWidth, unsigned int windowHeight, std::string windowTitle) {
	InitSfml(windowWidth, windowHeight, windowTitle);
	glCheckError();
	InitGlew();
	glCheckError();
	PrepareToRendering();
	glCheckError();
}

Render::~Render() {
	delete window;
}

void Render::InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle) {
	LOG(INFO) << "Creating window: " << WinWidth << "x" << WinHeight << " \"" << WinTitle << "\"";
	sf::ContextSettings contextSetting;
	contextSetting.majorVersion = 3;
	contextSetting.minorVersion = 3;
	contextSetting.attributeFlags = contextSetting.Core;
	contextSetting.depthBits = 24;
	window = new sf::Window(sf::VideoMode(WinWidth, WinHeight), WinTitle, sf::Style::Default, contextSetting);
	glCheckError();
	window->setPosition(sf::Vector2i(sf::VideoMode::getDesktopMode().width / 2 - window->getSize().x / 2,
	                                 sf::VideoMode::getDesktopMode().height / 2 - window->getSize().y / 2));
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
	glViewport(0, 0, window->getSize().x, window->getSize().y);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCheckError();
}

void Render::PrepareToRendering() {
	//TextureAtlas texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, AssetManager::Instance().GetTextureAtlas());	
}

void Render::RenderFrame() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (renderWorld)
        world->Render(renderState);

	window->display();
}

void Render::HandleEvents() {
	sf::Event event;
	while (window->pollEvent(event)) {
		switch (event.type) {
			case sf::Event::Closed:
				LOG(INFO) << "Received close event by window closing";
				isRunning = false;
				break;
			case sf::Event::Resized:
				glViewport(0, 0, window->getSize().x, window->getSize().y);
                renderState.WindowWidth = window->getSize().x;
                renderState.WindowHeight = window->getSize().y;
				break;
			case sf::Event::KeyPressed:
				if (!window->hasFocus()) break;
                switch (event.key.code) {
                case sf::Keyboard::Escape:
                    LOG(INFO) << "Received close event by esc";
                    isRunning = false;
                    break;
                    case sf::Keyboard::T:
                        SetMouseCapture(!isMouseCaptured);
                        break;
                    case sf::Keyboard::U:
                        EventAgregator::PushEvent(EventType::ConnectToServer, ConnectToServerData{ "127.0.0.1", 25565 });
                        break;
                    case sf::Keyboard::I:
                        EventAgregator::PushEvent(EventType::Disconnect, DisconnectData{ "Manual disconnect" });
                        break;
                    case sf::Keyboard::K:
                        if (renderWorld) {
                            world->MaxRenderingDistance--;
                            if (world->MaxRenderingDistance <= 0)
                                world->MaxRenderingDistance = 1;
                            LOG(INFO) << "Decreased rendering distance: " << world->MaxRenderingDistance;
                            EventAgregator::PushEvent(EventType::UpdateSectionsRender, UpdateSectionsRenderData{});
                        }
                    break;
                    case sf::Keyboard::L:
                        if (renderWorld) {
                            world->MaxRenderingDistance++;
                            LOG(INFO) << "Increased rendering distance: " << world->MaxRenderingDistance;
                            EventAgregator::PushEvent(EventType::UpdateSectionsRender, UpdateSectionsRenderData{});
                        }
                        break;
                    case sf::Keyboard::W:
                        EventAgregator::PushEvent(EventType::KeyPressed, KeyPressedData{ sf::Keyboard::W });
                        break;
                    case sf::Keyboard::A:
                        EventAgregator::PushEvent(EventType::KeyPressed, KeyPressedData{ sf::Keyboard::A });
                        break;
                    case sf::Keyboard::S:
                        EventAgregator::PushEvent(EventType::KeyPressed, KeyPressedData{ sf::Keyboard::S });
                        break;
                    case sf::Keyboard::D:
                        EventAgregator::PushEvent(EventType::KeyPressed, KeyPressedData{ sf::Keyboard::D });
                        break;
					default:
						break;
				}
            case sf::Event::KeyReleased:
                if (!window->hasFocus()) break;
                switch (event.key.code) {
                    case sf::Keyboard::W:
                        EventAgregator::PushEvent(EventType::KeyReleased, KeyReleasedData{ sf::Keyboard::W });
                        break;
                    case sf::Keyboard::A:
                        EventAgregator::PushEvent(EventType::KeyReleased, KeyReleasedData{ sf::Keyboard::A });
                        break;
                    case sf::Keyboard::S:
                        EventAgregator::PushEvent(EventType::KeyReleased, KeyReleasedData{ sf::Keyboard::S });
                        break;
                    case sf::Keyboard::D:
                        EventAgregator::PushEvent(EventType::KeyReleased, KeyReleasedData{ sf::Keyboard::D });
                        break;
                    default:
                        break;
                }
			default:
				break;
		}
	}	
}

void Render::HandleMouseCapture() {
	sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
	sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
	sf::Mouse::setPosition(center, *window);
	mouseXDelta = (mousePos - center).x, mouseYDelta = (center - mousePos).y;
	const float Sensetivity = 0.7f;
    EventAgregator::DirectEventCall(EventType::MouseMoved, MouseMovedData{ mouseXDelta * Sensetivity, mouseYDelta });	
}

void Render::SetMouseCapture(bool IsCaptured) {
	window->setMouseCursorVisible(!isMouseCaptured);
	sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
	isMouseCaptured = IsCaptured;
	window->setMouseCursorVisible(!IsCaptured);
}

void Render::ExecuteRenderLoop() {
	EventListener listener;

	listener.RegisterHandler(EventType::ConnectionSuccessfull, [this](EventData eventData) {
		auto data = std::get<ConnectionSuccessfullData>(eventData);
		window->setTitle("Connected");
	});

	listener.RegisterHandler(EventType::PlayerConnected, [this](EventData eventData) {
		auto data = std::get<PlayerConnectedData>(eventData);
		window->setTitle("Joined the game");
        world = std::make_unique<RendererWorld>(data.ptr);
	});

	listener.RegisterHandler(EventType::RemoveLoadingScreen, [this](EventData eventData) {
		window->setTitle("Playing");
        renderWorld = true;
	});

    listener.RegisterHandler(EventType::ConnectionFailed, [this](EventData eventData) {
        window->setTitle("Connection failed: " + std::get<ConnectionFailedData>(eventData).reason);
        renderWorld = false;
        world.reset();
    });

    listener.RegisterHandler(EventType::Disconnected, [this](EventData eventData) {
        window->setTitle("Disconnected: " + std::get<DisconnectedData>(eventData).reason);
        renderWorld = false;
        world.reset();
    });

    listener.RegisterHandler(EventType::Connecting, [this](EventData eventData) {
        window->setTitle("Connecting");
    });
    	
	LoopExecutionTimeController timer(std::chrono::milliseconds(16));
	while (isRunning) {
		HandleEvents();
		if (isMouseCaptured) HandleMouseCapture();
		glCheckError();

		RenderFrame();
		while (listener.IsEventsQueueIsNotEmpty())
			listener.HandleEvent();
        if (renderWorld)
            window->setTitle("FPS: "+std::to_string(1.0/timer.GetDeltaMs()*1000.0));
		timer.Update();
	}
    EventAgregator::PushEvent(EventType::Exit, ExitData{});
}
