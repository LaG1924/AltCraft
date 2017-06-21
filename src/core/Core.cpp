#include "Core.hpp"

Core::Core() {
	LOG(INFO) << "Core initializing...";
	InitSfml(900, 450, "AltCraft");
	glCheckError();
	InitGlew();
	glCheckError();
	client = new NetworkClient("127.0.0.1", 25565, "HelloOne", isRunning);
	gameState = new GameState(client, isRunning);
	std::thread loop = std::thread(&Core::UpdateGameState, this);
	std::swap(loop, gameStateLoopThread);
	assetManager = new AssetManager;
	PrepareToRendering();
	LOG(INFO) << "Core is initialized";
	glCheckError();
}

Core::~Core() {
	LOG(INFO) << "Core stopping...";
	gameStateLoopThread.join();
	delete shader;
	delete gameState;
	delete client;
	delete assetManager;
	delete window;
	LOG(INFO) << "Core is stopped";
}

void Core::Exec() {
	LOG(INFO) << "Main loop is executing!";
	isRunning = true;
	while (isRunning) {
		static sf::Clock clock, clock1;
		deltaTime = clock.getElapsedTime().asSeconds();
		absTime = clock1.getElapsedTime().asSeconds();
		clock.restart();

		static bool alreadyDone = false;
		if (gameState->g_IsGameStarted && !alreadyDone) {
			alreadyDone = true;
			UpdateChunksToRender();
		}

		std::ostringstream toWindow;
		auto camPos = gameState->Position();
		auto velPos = glm::vec3(gameState->g_PlayerVelocityX, gameState->g_PlayerVelocityY,
		                        gameState->g_PlayerVelocityZ);
		toWindow << std::setprecision(2) << std::fixed;
		toWindow << "Pos: " << camPos.x << ", " << camPos.y - 1.12 << ", " << camPos.z << "; ";
		toWindow << "Health: " << gameState->g_PlayerHealth << "; ";
		//toWindow << "OG: " << gameState->g_OnGround << "; ";
		toWindow << "Vel: " << velPos.x << ", " << velPos.y << ", " << velPos.z << "; ";
		toWindow << "FPS: " << (1.0f / deltaTime) << " ";
		toWindow << " (" << deltaTime * 1000 << "ms); ";
		toWindow << "Tickrate: " << tickRate << " (" << (1.0 / tickRate * 1000) << "ms); ";
		window->setTitle(toWindow.str());

		HandleEvents();
		if (isMouseCaptured)
			HandleMouseCapture();
		glCheckError();

		RenderFrame();

	}
}

void Core::RenderFrame() {
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	switch (currentState) {
		case MainMenu:
			//RenderGui(MenuScreen);
			break;
		case Loading:
			//RenderGui(LoadingScreen);
			break;
		case Playing:
			RenderWorld();
			//RenderGui(HUD);
			break;
		case PauseMenu:
			RenderWorld();
			//RenderGui(PauseGui);
			break;
	}

	window->display();
}

void Core::InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle) {
	LOG(INFO) << "Creating window: " << WinWidth << "x" << WinHeight << " \"" << WinTitle << "\"";
	sf::ContextSettings contextSetting;
	contextSetting.majorVersion = 3;
	contextSetting.minorVersion = 3;
	contextSetting.attributeFlags = contextSetting.Core;
	contextSetting.depthBits = 24;
	window = new sf::Window(sf::VideoMode(WinWidth, WinHeight), WinTitle, sf::Style::Default, contextSetting);
	glCheckError();
	window->setVerticalSyncEnabled(true);
	//window->setPosition(sf::Vector2i(sf::VideoMode::getDesktopMode().width / 2, sf::VideoMode::getDesktopMode().height / 2));
	window->setPosition(sf::Vector2i(sf::VideoMode::getDesktopMode().width / 2 - window->getSize().x / 2,
	                                 sf::VideoMode::getDesktopMode().height / 2 - window->getSize().y / 2));

	SetMouseCapture(false);
}

void Core::InitGlew() {
	LOG(INFO) << "Initializing GLEW";
	glewExperimental = GL_TRUE;
	GLenum glewStatus = glewInit();
	glCheckError();
	if (glewStatus != GLEW_OK) {
		LOG(FATAL) << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus);
	}
	glViewport(0, 0, width(), height());
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glCheckError();
}

unsigned int Core::width() {
	return window->getSize().x;
}

unsigned int Core::height() {
	return window->getSize().y;
}

void Core::HandleEvents() {
	sf::Event event;
	while (window->pollEvent(event)) {
		switch (event.type) {
			case sf::Event::Closed:
				LOG(INFO) << "Received close event by window closing";
				isRunning = false;
				break;
			case sf::Event::Resized:
				glViewport(0, 0, width(), height());
				break;
			case sf::Event::KeyPressed:
				if (!window->hasFocus())
					break;
				switch (event.key.code) {
					case sf::Keyboard::Escape:
						LOG(INFO) << "Received close event by esc";
						isRunning = false;
						break;
					case sf::Keyboard::T:
						SetMouseCapture(!isMouseCaptured);
						break;
					case sf::Keyboard::L:
						ChunkDistance++;
						LOG(INFO) << "Increased render distance: " << ChunkDistance;
						break;
					case sf::Keyboard::K:
						if (ChunkDistance > 1) {
							ChunkDistance--;
							LOG(INFO) << "Decreased render distance: " << ChunkDistance;
						}
						break;
					default:
						break;
				}
				/*case sf::Event::MouseWheelScrolled:
					if (!window->hasFocus())
						break;
					camera.ProcessMouseScroll(event.mouseWheelScroll.delta);
					break;*/
			default:
				break;
		}
	}
	if (window->hasFocus()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			gameState->HandleMovement(GameState::FORWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			gameState->HandleMovement(GameState::BACKWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			gameState->HandleMovement(GameState::LEFT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			gameState->HandleMovement(GameState::RIGHT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
			gameState->HandleMovement(GameState::JUMP, deltaTime);
		UpdateChunksToRender();
	}
}

void Core::HandleMouseCapture() {
	sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
	sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
	sf::Mouse::setPosition(center, *window);
	mouseXDelta = (mousePos - center).x, mouseYDelta = (center - mousePos).y;
	const float Sensetivity = 0.7f;
	gameState->HandleRotation(mouseXDelta * Sensetivity, mouseYDelta * Sensetivity);
	//camera.ProcessMouseMovement(mouseXDelta, mouseYDelta);
}

void Core::RenderWorld() {
	shader->Use();
	glCheckError();

	GLint projectionLoc = glGetUniformLocation(shader->Program, "projection");
	GLint viewLoc = glGetUniformLocation(shader->Program, "view");
	GLint timeLoc = glGetUniformLocation(shader->Program, "time");
	glm::mat4 projection = glm::perspective(45.0f, (float) width() / (float) height(), 0.1f, 10000000.0f);
	glm::mat4 view = gameState->GetViewMatrix();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(timeLoc, absTime);
	glUniform2f(glGetUniformLocation(shader->Program, "windowSize"), width(), height());

	glCheckError();

	for (auto &render : toRender) {
		Section &section = *availableChunks.find(render)->second.GetSection();

		std::vector<Vector> sectionCorners = {
				Vector(0, 0, 0),
				Vector(0, 0, 16),
				Vector(0, 16, 0),
				Vector(0, 16, 16),
				Vector(16, 0, 0),
				Vector(16, 0, 16),
				Vector(16, 16, 0),
				Vector(16, 16, 16),
		};
		bool isBreak = true;
		for (auto &it:sectionCorners) {
			glm::mat4 vp = projection * view;
			glm::vec3 point(section.GetPosition().GetX() * 16 + it.GetX(),
			                section.GetPosition().GetY() * 16 + it.GetY(),
			                section.GetPosition().GetZ() * 16 + it.GetZ());
			glm::vec4 p = vp * glm::vec4(point, 1);
			glm::vec3 res = glm::vec3(p) / p.w;
			if (res.x < 1 && res.x > -1 && res.y < 1 && res.y > -1 && res.z > 0) {
				isBreak = false;
				break;
			}
		}
		if (isBreak && glm::length(gameState->Position() -
		                           glm::vec3(section.GetPosition().GetX() * 16,
		                                     section.GetPosition().GetY() * 16,
		                                     section.GetPosition().GetZ() * 16)) > 30.0f) {
			continue;
		}
		availableChunks.find(render)->second.Render(renderState);
	}
	glCheckError();
}

void Core::SetMouseCapture(bool IsCaptured) {
	window->setMouseCursorVisible(!isMouseCaptured);
	sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
	isMouseCaptured = IsCaptured;
	window->setMouseCursorVisible(!IsCaptured);
}

void Core::PrepareToRendering() {
	shader = new Shader("./shaders/block.vs", "./shaders/block.fs");
	shader->Use();

	LOG(INFO) << "Initializing texture atlas...";
	//TextureAtlas texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, assetManager->GetTextureAtlas());
	glUniform1i(glGetUniformLocation(shader->Program, "textureAtlas"), 0);

	//TextureAtlas coordinates
	std::vector<glm::vec4> textureCoordinates;
	std::vector<GLint> indexes;
	GLint totalTextures;
	for (int id = 1; id < 128; id++) {
		for (int state = 0; state < 16; state++) {
			BlockTextureId blockTextureId(id, state, 6);
			if (!assetManager->GetTextureByBlock(blockTextureId) &&
			    !assetManager->GetTextureByBlock(BlockTextureId(id, state, 0))) {
				continue;
			}
			int side = assetManager->GetTextureByBlock(blockTextureId) ? 6 : 0;
			do {
				int index = (side << 16) | (id << 4) | state;
				TextureCoordinates tc = assetManager->GetTextureByBlock(BlockTextureId(id, state, side));
				textureCoordinates.push_back(glm::vec4(tc.x, tc.y, tc.w, tc.h));
				indexes.push_back(index);
				side++;
			} while (side < 6);
		}
	}
	totalTextures = indexes.size();
	LOG(INFO) << "Created " << totalTextures << " texture indexes";
	CHECK_EQ(indexes.size(), textureCoordinates.size())
		<< "Arrays of textureCoordinates and of indexes is not equals";
	CHECK_LE(totalTextures, 1023) << "There is more texture indexes, than GLSL buffer allows";

	GLuint bp1 = 0;
	GLuint ubo = glGetUniformBlockIndex(shader->Program, "TextureIndexes");
	glUniformBlockBinding(shader->Program, ubo, bp1);
	glGenBuffers(1, &UboTextureIndexes);
	glBindBuffer(GL_UNIFORM_BUFFER, UboTextureIndexes);
	glBindBufferBase(GL_UNIFORM_BUFFER, bp1, UboTextureIndexes);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + sizeof(glm::vec4) * 1023, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLint), &totalTextures); //copy totalTextures
	for (size_t i = 0; i < indexes.size(); i++) {
		size_t baseOffset = sizeof(glm::vec4);
		size_t itemOffset = sizeof(glm::vec4);
		size_t offset = baseOffset + i * itemOffset;
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLint), &indexes[i]); //copy indexes' item
	}
	glCheckError();

	GLuint bp2 = 1;
	GLuint ubo2_index = glGetUniformBlockIndex(shader->Program, "TextureData");
	glUniformBlockBinding(shader->Program, ubo2_index, bp2);
	glGenBuffers(1, &UboTextureData);
	glBindBuffer(GL_UNIFORM_BUFFER, UboTextureData);
	glBindBufferBase(GL_UNIFORM_BUFFER, bp2, UboTextureData);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 1024, nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4) * textureCoordinates.size(), textureCoordinates.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glCheckError();
}

void Core::UpdateChunksToRender() {
	Vector playerChunk = Vector(floor(gameState->g_PlayerX / 16.0f), 0, floor(gameState->g_PlayerZ / 16.0f));
	static Vector previousPlayerChunk = playerChunk;
	static bool firstTime = true;
	static int previousRenderDistance = ChunkDistance;
	if (previousPlayerChunk == playerChunk && !firstTime && ChunkDistance == previousRenderDistance) {
		return;
	}
	previousPlayerChunk = playerChunk;
	previousRenderDistance = ChunkDistance;
	toRender.clear();
	for (auto &it:gameState->world.sections) {
		Vector chunkPosition = it.first;
		chunkPosition.SetY(0);
		Vector delta = chunkPosition - playerChunk;
		if (delta.GetMagnitude() > ChunkDistance)
			continue;
		toRender.push_back(it.first);
	}
	for (auto &it:toRender) {
		if (availableChunks.find(it) == availableChunks.end()) {
			auto pair = std::make_pair(it, RenderSection(&gameState->world, it));
			availableChunks.insert(pair);
		} else {
			availableChunks.find(it)->second.UpdateState();
		}
	}
	if (firstTime)
		LOG(INFO) << "Chunks to render: " << toRender.size() << " of " << availableChunks.size();
	firstTime = false;
}

void Core::UpdateGameState() {
	el::Helpers::setThreadName("Game");
	LOG(INFO) << "GameState thread is started";
	sf::Clock delta;
	while (isRunning) {
		float deltaTime = delta.getElapsedTime().asSeconds();
		delta.restart();
		gameState->Update(deltaTime);
		const double targetDelta = 1 / 60.0;
		std::chrono::duration<double, std::ratio<1, 1>> timeToSleep(targetDelta - delta.getElapsedTime().asSeconds());
		std::this_thread::sleep_for(timeToSleep);
		tickRate = 1 / delta.getElapsedTime().asSeconds();
	}
	LOG(INFO) << "GameState thread is stopped";
}