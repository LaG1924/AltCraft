#include "Core.hpp"

//Core::Core():toRenderMutex("toRender"),availableChunksMutex("availableChunks") {
Core::Core() {
	LOG(INFO) << "Core initializing...";
	InitSfml(900, 450, "AltCraft");
	glCheckError();
	InitGlew();
	glCheckError();
	client = new NetworkClient("127.0.0.1", 25565, "HelloOne", isRunning);
	gameState = new GameState(client, isRunning);
	gameStateLoopThread = std::thread(&Core::UpdateGameState, this);
	sectionUpdateLoopThread = std::thread(&Core::UpdateSections, this);
	assetManager = new AssetManager;
	PrepareToRendering();
	LOG(INFO) << "Core is initialized";
	glCheckError();
}

Core::~Core() {
	LOG(INFO) << "Core stopping...";
	gameStateLoopThread.join();
	sectionUpdateLoopThread.join();
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

		/*std::ostringstream toWindow;
		auto camPos = gameState->Position();
		auto velPos = glm::vec3(gameState->g_PlayerVelocityX, gameState->g_PlayerVelocityY,
		                        gameState->g_PlayerVelocityZ);
		toWindow << std::setprecision(2) << std::fixed;
		toWindow << "Pos: " << camPos.x << ", " << camPos.y << ", " << camPos.z << "; ";
		toWindow << "Health: " << gameState->g_PlayerHealth << "; ";
		//toWindow << "OG: " << gameState->g_OnGround << "; ";
		toWindow << "Vel: " << velPos.x << ", " << velPos.y << ", " << velPos.z << "; ";
		toWindow << "FPS: " << (1.0f / deltaTime) << " ";
		toWindow << " (" << deltaTime * 1000 << "ms); ";
		toWindow << "Tickrate: " << tickRate << " (" << (1.0 / tickRate * 1000) << "ms); ";
        toWindow << "Sections: " << sectionRate << " (" << (1.0 / sectionRate * 1000) << "ms); ";
		window->setTitle(toWindow.str());*/

		HandleEvents();
		if (isMouseCaptured) HandleMouseCapture();
		glCheckError();

		RenderFrame();
		if (isRendersShouldBeCreated) {
			availableChunksMutex.lock();
			for (auto &it:renders) {
				auto pair = std::make_pair(it, RenderSection(&gameState->world, it));
				availableChunks.insert(pair);
			}
			renders.clear();
			availableChunksMutex.unlock();
			isRendersShouldBeCreated = false;
			waitRendersCreated.notify_all();
		}

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
	//window->setVerticalSyncEnabled(true);
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
	glEnable(GL_CULL_FACE);
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
				if (!window->hasFocus()) break;
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
			default:
				break;
		}
	}
	if (window->hasFocus()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) gameState->HandleMovement(GameState::FORWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) gameState->HandleMovement(GameState::BACKWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) gameState->HandleMovement(GameState::LEFT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) gameState->HandleMovement(GameState::RIGHT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) gameState->HandleMovement(GameState::JUMP, deltaTime);
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

	toRenderMutex.lock();
	for (auto &render : toRender) {
		availableChunksMutex.lock();
		auto iterator = availableChunks.find(render);
        if (iterator == availableChunks.end()) {
            availableChunksMutex.unlock();
            continue;
        }
		/*Section &section = *iterator->second.GetSection();
		//availableChunksMutex.unlock();

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
		glm::mat4 vp = projection * view;
		for (auto &it:sectionCorners) {
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
            availableChunksMutex.unlock();
			continue;
		}
		//availableChunksMutex.lock();*/
		iterator->second.Render(renderState);
		availableChunksMutex.unlock();
	}
	toRenderMutex.unlock();
	glCheckError();
}

void Core::SetMouseCapture(bool IsCaptured) {
	window->setMouseCursorVisible(!isMouseCaptured);
	sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
	isMouseCaptured = IsCaptured;
	window->setMouseCursorVisible(!IsCaptured);
}

void Core::PrepareToRendering() {
	shader = new Shader("./shaders/face.vs", "./shaders/face.fs");
	shader->Use();

	//TextureAtlas texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, assetManager->GetTextureAtlas());
	glUniform1i(glGetUniformLocation(shader->Program, "textureAtlas"), 0);
}

void Core::UpdateChunksToRender() {
	return;
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
	for (auto &it:gameState->world.GetSectionsList()) {
		Vector chunkPosition = it;
		chunkPosition.SetY(0);
		Vector delta = chunkPosition - playerChunk;
		if (delta.GetMagnitude() > ChunkDistance) continue;
		toRender.push_back(it);
	}
	for (auto &it:toRender) {
		if (availableChunks.find(it) == availableChunks.end()) {
			auto pair = std::make_pair(it, RenderSection(&gameState->world, it));
			pair.second.UpdateState(assetManager->GetTextureAtlasIndexes());
			availableChunks.insert(pair);
		} else {
			//availableChunks.find(it)->second.UpdateState();
		}
	}
	if (firstTime) LOG(INFO) << "Chunks to render: " << toRender.size() << " of " << availableChunks.size();
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

void Core::UpdateSections() {
	glm::vec3 playerPosition = gameState->Position();
	float playerPitch = gameState->Pitch();
	float playerYaw = gameState->Yaw();
    sf::Clock delta;
    std::vector<Vector> chunksToRender;
    auto currentSectionIterator = chunksToRender.begin();
	while (isRunning) {
        delta.restart();        
		if (glm::length(glm::distance(gameState->Position(), playerPosition)) > 5.0f) {
            chunksToRender.clear();
			playerPosition = gameState->Position();
			Vector playerChunk = Vector(floor(playerPosition.x / 16.0f), 0, floor(playerPosition.z / 16.0f));
			for (auto &it:gameState->world.GetSectionsList()) {
				Vector chunkPosition = it;
				chunkPosition.SetY(0);
				Vector delta = chunkPosition - playerChunk;
				if (delta.GetMagnitude() > ChunkDistance) continue;
				chunksToRender.push_back(it);
			}
            std::sort(chunksToRender.begin(), chunksToRender.end(), [playerChunk](auto first, auto second) {
                glm::vec3 fDistance = first - playerChunk;
                glm::vec3 sDistance = second - playerChunk;
                return glm::length(fDistance) < glm::length(sDistance);
            });
			for (auto &it:chunksToRender) {
				availableChunksMutex.lock();
				if (availableChunks.find(it) == availableChunks.end()) {
					availableChunksMutex.unlock();
					renders.push_back(it);
				} else
                    availableChunksMutex.unlock();
			}
			if (!renders.empty()) {
				std::mutex mutex;
				std::unique_lock<std::mutex> lock(mutex);
				isRendersShouldBeCreated = true;
				while (isRendersShouldBeCreated)
					waitRendersCreated.wait(lock);
			}
            currentSectionIterator = chunksToRender.begin();
			toRenderMutex.lock();
			toRender = chunksToRender;
			toRenderMutex.unlock();
		}
        if (currentSectionIterator != chunksToRender.end()) {
            availableChunksMutex.lock();
            auto iterator = availableChunks.find(*currentSectionIterator);
            if (iterator != availableChunks.end() && iterator->second.IsNeedUpdate()) {
                RenderSection rs = std::move(iterator->second);
                availableChunks.erase(iterator);
                auto pair = std::make_pair(*currentSectionIterator, rs);
                availableChunksMutex.unlock();

                pair.second.UpdateState(assetManager->GetTextureAtlasIndexes());

                availableChunksMutex.lock();
                availableChunks.insert(pair);
            }
            availableChunksMutex.unlock();
            currentSectionIterator = std::next(currentSectionIterator);
        }
		if (gameState->Pitch() != playerPitch || gameState->Yaw() != playerYaw) {
			playerPitch = gameState->Pitch();
			playerYaw = gameState->Yaw();
            const std::vector<Vector> sectionCorners = {
                Vector(0, 0, 0),
                Vector(0, 0, 16),
                Vector(0, 16, 0),
                Vector(0, 16, 16),
                Vector(16, 0, 0),
                Vector(16, 0, 16),
                Vector(16, 16, 0),
                Vector(16, 16, 16),
            };
            const glm::mat4 projection = glm::perspective(45.0f, (float)width() / (float)height(), 0.1f, 10000000.0f);
            const glm::mat4 view = gameState->GetViewMatrix();
            const glm::mat4 vp = projection * view;
            for (auto& section: toRender) {
                bool isCulled = true;                
                for (auto &it : sectionCorners) {
                    glm::vec3 point(section.GetX() * 16 + it.GetX(), 
                                    section.GetY() * 16 + it.GetY(),
                                    section.GetZ() * 16 + it.GetZ());
                    glm::vec4 p = vp * glm::vec4(point, 1);
                    glm::vec3 res = glm::vec3(p) / p.w;
                    if (res.x < 1 && res.x > -1 && res.y < 1 && res.y > -1 && res.z > 0) {
                        isCulled = false;
                        break;
                    }
                }
                bool isVisible = !isCulled || glm::length(gameState->Position() -
                    glm::vec3(section.GetX() * 16, section.GetY() * 16, section.GetZ() * 16)) < 30.0f;      
                availableChunksMutex.lock();
                auto iter = availableChunks.find(section);
                if (iter != availableChunks.end())
                    iter->second.SetEnabled(isVisible);
                availableChunksMutex.unlock();
                
            }
		}
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(5ms);
        sectionRate = delta.getElapsedTime().asSeconds();
        delta.restart();
	}
}

MyMutex::MyMutex(std::string name) {
    str = name;
}

void MyMutex::lock() {
    LOG(WARNING) << "Thread " << std::this_thread::get_id() << " locked mutex " << str;
    mtx.lock();
}

void MyMutex::unlock() {
    LOG(WARNING) << "Thread " << std::this_thread::get_id() << " unlocked mutex " << str;
    mtx.unlock();
}