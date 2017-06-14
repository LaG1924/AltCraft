#include "Core.hpp"

GLenum glCheckError_(const char *file, int line) {
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
			case GL_INVALID_ENUM:
				error = "INVALID_ENUM";
				break;
			case GL_INVALID_VALUE:
				error = "INVALID_VALUE";
				break;
			case GL_INVALID_OPERATION:
				error = "INVALID_OPERATION";
				break;
			case GL_STACK_OVERFLOW:
				error = "STACK_OVERFLOW";
				break;
			case GL_STACK_UNDERFLOW:
				error = "STACK_UNDERFLOW";
				break;
			case GL_OUT_OF_MEMORY:
				error = "OUT_OF_MEMORY";
				break;
			case GL_INVALID_FRAMEBUFFER_OPERATION:
				error = "INVALID_FRAMEBUFFER_OPERATION";
				break;
		}
		LOG(ERROR) << "OpenGL error: " << error << " at " << file << ":" << line;
	}
	return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

const GLfloat vertices[] = {
		//Z+ edge
		-0.5f, 0.5f, 0.5f,
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,

		//Z- edge
		-0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,

		//X+ edge
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
		-0.5f, 0.5f, 0.5f,

		//X- edge
		0.5f, -0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, 0.5f, -0.5f,

		//Y+ edge
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, 0.5f,
		0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, -0.5f,
		-0.5f, 0.5f, 0.5f,

		//Y- edge
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, 0.5f,
		-0.5f, -0.5f, -0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, 0.5f,
};

const GLfloat uv_coords[] = {
		//Z+
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,

		//Z-
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,

		//X+
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,

		//X-
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,

		//Y+
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,

		//Y-
		1.0f, 0.0f,
		0.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
};

Core::Core() {
	LOG(INFO) << "Core initializing...";
	InitSfml(900, 450, "AltCraft");
	glCheckError();
	InitGlew();
	glCheckError();
	client = new NetworkClient("127.0.0.1", 25565, "HelloOne");
	gameState = new GameState(client);
	std::thread loop = std::thread(&Core::UpdateGameState, this);
	std::swap(loop, gameStateLoopThread);
	assetManager = new AssetManager;
	PrepareToWorldRendering();
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
		glm::highp_vec3 camPos(camera.Position);
		toWindow << std::setprecision(2) << std::fixed;
		toWindow << "Pos: " << camPos.x << ", " << camPos.y << ", " << camPos.z << "; ";
		toWindow << "FPS: " << (1.0f / deltaTime) << " ";
		toWindow << " (" << deltaTime * 1000 << "ms) ";
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
	//glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
					case sf::Keyboard::Z:
						camera.MovementSpeed /= 2;
						break;
					case sf::Keyboard::X:
						camera.MovementSpeed *= 2;
						break;
					case sf::Keyboard::M:
						std::sort(toRender.begin(), toRender.end(), [this](const Vector &lhs, const Vector &rhs) {
							return glm::length((glm::vec3) lhs - camera.Position) <
							       glm::length((glm::vec3) rhs - camera.Position);
						});
						LOG(WARNING) << "Render list is optimized";
						break;
					case sf::Keyboard::K:
						ChunkDistance++;
						LOG(INFO)<<"Increased render distance: "<<ChunkDistance;
						break;
					case sf::Keyboard::L:
						ChunkDistance--;
						LOG(INFO)<<"Decreased render distance: "<<ChunkDistance;
						break;
					case sf::Keyboard::O:
						UpdateChunksToRender();
						LOG(INFO)<<"Render list is updated";
						break;
					default:
						break;
				}
			case sf::Event::MouseWheelScrolled:
				if (!window->hasFocus())
					break;
				camera.ProcessMouseScroll(event.mouseWheelScroll.delta);
				break;
			default:
				break;
		}
	}
	if (window->hasFocus()) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
			camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
			camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
			camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
			camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
	}
}

void Core::HandleMouseCapture() {
	sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
	sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
	sf::Mouse::setPosition(center, *window);
	mouseXDelta = (mousePos - center).x, mouseYDelta = (center - mousePos).y;
	camera.ProcessMouseMovement(mouseXDelta, mouseYDelta);
}

void Core::RenderGui(Gui &Target) {
	Target.WHY++;
}

void Core::RenderWorld() {
	shader->Use();
	glCheckError();

	GLint modelLoc = glGetUniformLocation(shader->Program, "model");
	GLint projectionLoc = glGetUniformLocation(shader->Program, "projection");
	GLint viewLoc = glGetUniformLocation(shader->Program, "view");
	GLint blockLoc = glGetUniformLocation(shader->Program, "Block");
	GLint stateLoc = glGetUniformLocation(shader->Program, "State");
	GLint timeLoc = glGetUniformLocation(shader->Program, "time");
	glm::mat4 projection = glm::perspective(camera.Zoom, (float) width() / (float) height(), 0.1f, 10000000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
	glUniform1f(timeLoc, absTime);
	glUniform2f(glGetUniformLocation(shader->Program, "windowSize"), width(), height());

	glCheckError();

	glBindVertexArray(VAO);
	for (auto &sectionPos : toRender) {
		Section &section = gameState->world.m_sections.find(sectionPos)->second;

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
			glm::vec3 point(sectionPos.GetX() * 16 + it.GetX(), sectionPos.GetY() * 16 + it.GetY(),
			                sectionPos.GetZ() * 16 + it.GetZ());
			glm::vec4 p = vp * glm::vec4(point, 1);
			glm::vec3 res = glm::vec3(p) / p.w;
			if (res.x < 1 && res.x > -1 && res.y < 1 && res.y > -1 && res.z > 0) {
				isBreak = false;
				break;
			}
		}
		if (isBreak && glm::length(
				camera.Position - glm::vec3(sectionPos.GetX() * 16, sectionPos.GetY() * 16, sectionPos.GetZ() * 16)) >
		               30) {
			//zLOG(ERROR)<<"CULL";
			continue;
		}

		std::vector<glm::mat4> &arrOfModels = toRenderModels[sectionPos];
		std::vector<glm::vec2> &arrOfBlocks = toRenderBlocks[sectionPos];

		glBindBuffer(GL_ARRAY_BUFFER, VBO3);
		glBufferData(GL_ARRAY_BUFFER, arrOfModels.size() * sizeof(glm::mat4), arrOfModels.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, VBO4);
		glBufferData(GL_ARRAY_BUFFER, arrOfBlocks.size() * sizeof(glm::vec2), arrOfBlocks.data(), GL_DYNAMIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, arrOfModels.size());
	}
	glBindVertexArray(0);
	glCheckError();
}

void Core::SetMouseCapture(bool IsCaptured) {
	window->setMouseCursorVisible(!isMouseCaptured);
	sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
	isMouseCaptured = IsCaptured;
	window->setMouseCursorVisible(!IsCaptured);
}

void Core::PrepareToWorldRendering() {
	//Cube-rendering data
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &VBO3);
	glGenBuffers(1, &VBO4);
	glGenVertexArrays(1, &VAO);

	glBindVertexArray(VAO);
	{
		//Cube vertices
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);

		//Cube UVs
		glBindBuffer(GL_ARRAY_BUFFER, VBO2);
		glBufferData(GL_ARRAY_BUFFER, sizeof(uv_coords), uv_coords, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(2);

		//Blocks ids
		glBindBuffer(GL_ARRAY_BUFFER, VBO4);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * 16 * 16 * 16, NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(7);
		glVertexAttribDivisor(7, 1);
		glCheckError();

		//Blocks models
		glBindBuffer(GL_ARRAY_BUFFER, VBO3);
		glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * 16 * 16 * 16, NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(8);
		glVertexAttribPointer(8 + 1, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat),
		                      (void *) (1 * 4 * sizeof(GLfloat)));
		glEnableVertexAttribArray(8 + 1);
		glVertexAttribPointer(8 + 2, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat),
		                      (void *) (2 * 4 * sizeof(GLfloat)));
		glEnableVertexAttribArray(8 + 2);
		glVertexAttribPointer(8 + 3, 4, GL_FLOAT, GL_FALSE, 4 * 4 * sizeof(GLfloat),
		                      (void *) (3 * 4 * sizeof(GLfloat)));
		glEnableVertexAttribArray(8 + 3);
		glVertexAttribDivisor(8, 1);
		glVertexAttribDivisor(8 + 1, 1);
		glVertexAttribDivisor(8 + 2, 1);
		glVertexAttribDivisor(8 + 3, 1);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);

	glCheckError();

	shader = new Shader("./shaders/block.vs", "./shaders/block.fs");
	shader->Use();

	shader2 = new Shader("./shaders/simple.vs", "./shaders/simple.fs");

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
				/*LOG(ERROR) << "Encoded texture (" << id << " " << state << " " << side << ") as " << index << " ("
						   << std::bitset<19>(index) << ")" << " = " << tc.x << "," << tc.y << "," << tc.w << ","
						   << tc.h;*/
				/*LOG(FATAL)<<std::bitset<18>(index);
				side = 0x7;
				id = 0xFFF;
				state = 0xF;
				LOG(WARNING) << "side: " << side << " id: " << id << " state: " << state;
				int i, si, st, index = 0;
				si = side << 15;
				i = id<<3;
				st = state;
				index = i | si | st;
				LOG(FATAL) << std::bitset<18>(index) << " (" << index << "): " << std::bitset<18>(si) << " "
				<< std::bitset<18>(i) << " " << std::bitset<18>(st);*/
				/*if (rand() == 73) //Almost impossible(Almost==1/32768)
				{
				int index = 393233;
				LOG(WARNING) << std::bitset<20>(index) << "(" << index << ")";
				int side = (index & 0xE0000) >> 16;
				int id = (index & 0xFF0) >> 4;
				int state = index & 0xF;
				LOG(WARNING) << std::bitset<20>(side) << " " << std::bitset<20>(id) << " "
				<< std::bitset<20>(state);
				LOG(FATAL) << side << " " << id << " " << state;
				}*/
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
	glGenBuffers(1, &UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO);
	glBindBufferBase(GL_UNIFORM_BUFFER, bp1, UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) + sizeof(glm::vec4) * 1023, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(GLint), &totalTextures); //copy totalTextures
	for (int i = 0; i < indexes.size(); i++) {
		size_t baseOffset = sizeof(glm::vec4);
		size_t itemOffset = sizeof(glm::vec4);
		size_t offset = baseOffset + i * itemOffset;
		/*int index = indexes[i];
		int side = (index & 0x70000) >> 16;
		int id = (index & 0xFF0) >> 4;
		int state = index & 0xF;
		LOG(WARNING) << "Copying " << indexes[i] << " at " << offset<<" side is "<<side;*/
		glBufferSubData(GL_UNIFORM_BUFFER, offset, sizeof(GLint), &indexes[i]); //copy indexes' item
	}
	glCheckError();

	GLuint bp2 = 1;
	GLuint ubo2_index = glGetUniformBlockIndex(shader->Program, "TextureData");
	glUniformBlockBinding(shader->Program, ubo2_index, bp2);
	glGenBuffers(1, &UBO2);
	glBindBuffer(GL_UNIFORM_BUFFER, UBO2);
	glBindBufferBase(GL_UNIFORM_BUFFER, bp2, UBO2);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 1024, NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4) * textureCoordinates.size(), textureCoordinates.data());
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glCheckError();

	//Draw Lines preparing
	glGenBuffers(1, &VBO5);
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);
	{
		glBindBuffer(GL_ARRAY_BUFFER, VBO5);
		glBufferData(GL_ARRAY_BUFFER, 6 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	glBindVertexArray(0);
}

void Core::UpdateChunksToRender() {
	camera.Position = glm::vec3(gameState->g_PlayerX, gameState->g_PlayerY, gameState->g_PlayerZ);
	toRender.clear();
	Vector playerChunk = Vector(floor(gameState->g_PlayerX / 16.0f), 0, floor(gameState->g_PlayerZ / 16.0f));
	for (auto &it:gameState->world.m_sections) {
		Vector chunkPosition = it.first;
		chunkPosition.SetY(0);
		Vector delta = chunkPosition - playerChunk;
		if (delta.GetMagnitude() > ChunkDistance)
			continue;
		toRender.push_back(it.first);
	}
	LOG(INFO) << "Chunks to render: " << toRender.size();
	for (auto &it:toRender) {
		Section &section = gameState->world.m_sections.find(it)->second;
		std::vector<glm::mat4> models;
		std::vector<glm::vec2> blocks;
		for (int y = 0; y < 16; y++) {
			for (int z = 0; z < 16; z++) {
				for (int x = 0; x < 16; x++) {
					Block block = section.GetBlock(Vector(x, y, z));
					if (block.id == 0)
						continue;
					unsigned char isVisible = 0;
					if (x == 0 || x == 15 || y == 0 || y == 15 || z == 0 || z == 15) {
						isVisible = 0;
					} else {
						isVisible |= (section.GetBlock(Vector(x + 1, y, z)).id != 0) << 0;
						isVisible |= (section.GetBlock(Vector(x - 1, y, z)).id != 0) << 1;
						isVisible |= (section.GetBlock(Vector(x, y + 1, z)).id != 0) << 2;
						isVisible |= (section.GetBlock(Vector(x, y - 1, z)).id != 0) << 3;
						isVisible |= (section.GetBlock(Vector(x, y, z + 1)).id != 0) << 4;
						isVisible |= (section.GetBlock(Vector(x, y, z - 1)).id != 0) << 5;
					}
					if (isVisible == 0x3F)
						continue;
					glm::vec2 data(block.id, block.state);
					blocks.push_back(data);
					glm::mat4 model;
					model = glm::translate(model, glm::vec3(it.GetX() * 16, it.GetY() * 16, it.GetZ() * 16));
					model = glm::translate(model, glm::vec3(x, y, z));
					double size = 0.999;
					model = glm::scale(model, glm::vec3(size, size, size));
					models.push_back(model);
				}
			}
		}
		toRenderBlocks[it] = blocks;
		toRenderModels[it] = models;
	}
	std::sort(toRender.begin(), toRender.end(), [this](const Vector &lhs, const Vector &rhs) {
		return glm::length((glm::vec3) lhs - camera.Position) < glm::length((glm::vec3) rhs - camera.Position);
	});
	LOG(INFO) << "Chunks is prepared to rendering...";
}

void Core::UpdateGameState() {
	el::Helpers::setThreadName("Game");
	LOG(INFO) << "GameState thread is started";
	while (isRunning) {
		gameState->Update();
		if (toRender.size() > 0)
			break;
	}
	LOG(INFO) << "GameState thread is stopped";
}

void Core::DrawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color) {
	shader2->Use();
	glm::mat4 projection = glm::perspective(camera.Zoom, (float) width() / (float) height(), 0.1f, 10000000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader2->Program, "projection"), 1, GL_FALSE,
	                   glm::value_ptr(projection));
	glUniformMatrix4fv(glGetUniformLocation(shader2->Program, "view"), 1, GL_FALSE, glm::value_ptr(view));

	/*GLfloat data[6];
	data[0] = from[0];
	data[1] = from[1];
	data[2] = to[2];
	data[3] = to[0];
	data[4] = to[1];
	data[5] = from[2];*/
	GLfloat data[] = {0.5f, 0.5f, 0.0f,
	                  0.5f, -0.5f, 0.0f,
	                  -0.5f, 0.5f, 0.0f,

	                  0.5f, -0.5f, 0.0f,
	                  -0.5f, -0.5f, 0.0f,
	                  -0.5f, 0.5f, 0.0f,};

	glUniform3f(glGetUniformLocation(shader2->Program, "color"), color[0], color[1], color[2]);

	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO5);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 18, data, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glBindVertexArray(0);
	glEnable(GL_DEPTH_TEST);
}

void Core::UpdateOptimizedRender() {

	LOG(INFO) << "Render list is optimized";
}
