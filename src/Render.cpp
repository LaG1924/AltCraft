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
	shader = new Shader("./shaders/face.vs", "./shaders/face.fs");
	shader->Use();

	//TextureAtlas texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, AssetManager::Instance().GetTextureAtlas());
	glUniform1i(glGetUniformLocation(shader->Program, "textureAtlas"), 0);
}

void Render::RenderFrame() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
						EventAgregator::PushEvent(EventType::ConnectToServer, ConnectToServerData{"127.0.0.1", 25565});
					default:
						break;
				}
			default:
				break;
		}
	}
	if (window->hasFocus()) {
		/*if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) gameState->HandleMovement(GameState::FORWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) gameState->HandleMovement(GameState::BACKWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) gameState->HandleMovement(GameState::LEFT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) gameState->HandleMovement(GameState::RIGHT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) gameState->HandleMovement(GameState::JUMP, deltaTime);*/
	}
}

void Render::HandleMouseCapture() {
	sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
	sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
	sf::Mouse::setPosition(center, *window);
	mouseXDelta = (mousePos - center).x, mouseYDelta = (center - mousePos).y;
	const float Sensetivity = 0.7f;
	//gameState->HandleRotation(mouseXDelta * Sensetivity, mouseYDelta * Sensetivity);
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
	});

	listener.RegisterHandler(EventType::RemoveLoadingScreen, [this](EventData eventData) {
		window->setTitle("Loaded");
	});

	using namespace std::chrono_literals;
	LoopExecutionTimeController timer(16ms);
	while (isRunning) {
		HandleEvents();
		if (isMouseCaptured) HandleMouseCapture();
		glCheckError();

		RenderFrame();
		while (listener.IsEventsQueueIsNotEmpty())
			listener.HandleEvent();
		timer.Update();
	}
	EventData data = GlobalAppStateData{GlobalState::Exiting};
	EventAgregator::PushEvent(EventType::GlobalAppState, data);
}
