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

/*const GLfloat vertices[] = {
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
};*/
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
    InitSfml(1280, 720, "AltCraft");
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
    GLint blockLoc = glGetUniformLocation(shader->Program, "block");
    GLint timeLoc = glGetUniformLocation(shader->Program, "time");
    glm::mat4 projection = glm::perspective(camera.Zoom, (float) width() / (float) height(), 0.1f, 10000000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform1f(timeLoc, absTime);

    glCheckError();

    glBindVertexArray(VAO);

    for (auto &sectionPos:toRender) {
        Section &section = gameState->world.m_sections.find(sectionPos)->second;
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    Block block = section.GetBlock(Vector(x, y, z));
                    if (block.id == 0)
                        continue;

                    glm::mat4 model;
                    model = glm::translate(model, glm::vec3(sectionPos.GetX() * 16, sectionPos.GetY() * 16,
                                                            sectionPos.GetZ() * 16));
                    model = glm::translate(model, glm::vec3(x, y, z));

                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(blockLoc, block.id);

                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }
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
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO2);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uv_coords), uv_coords, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(2);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
    }
    glBindVertexArray(0);

    glCheckError();

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
    for (int id = 0; id < 4096; id++) {
        bool isReachedEnd = true;
        for (int state = 0; state < 16; state++) {
            if (!assetManager->GetTextureByBlock(BlockTextureId(id, state, 6)) ||
                !assetManager->GetTextureByBlock(BlockTextureId(id, state, 0))) {
                continue;
            }
            isReachedEnd = false;
            int side = assetManager->GetTextureByBlock(BlockTextureId(id, state, 6)) ? 6 : 0;
            do {
                int index = (side << 16) | (id << 4) | state;
                TextureCoordinates tc = assetManager->GetTextureByBlock(BlockTextureId(id, state, side));
                textureCoordinates.push_back(glm::vec4(tc.x, tc.y, tc.w, tc.h));
                indexes.push_back(index);
                /*LOG(ERROR) << "Encoded (" << side << " " << id << " " << state << ") as " << index << " ("
                           << std::bitset<20>(index) << ")";*/
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
            } while (side < 7);
        }
        if (isReachedEnd)
            break;

    }
    totalTextures = indexes.size();
    LOG(INFO) << "Created " << totalTextures << " texture indexes";
    CHECK_EQ(indexes.size(), textureCoordinates.size()) << "Arrays of textureCoordinates and of indexes is not equals";
    CHECK_LE(totalTextures, 2048) << "There is more texture indexes, than GLSL buffer allows";

    for (auto& it:indexes){
        LOG(WARNING)<<it;
    }

    indexes.insert(indexes.begin(), totalTextures);
    indexes.resize(2048);



    GLuint ubo = glGetUniformBlockIndex(shader->Program, "TextureIndexes");
    glUniformBlockBinding(shader->Program, ubo, 0);
    glGenBuffers(1, &UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, indexes.size() * sizeof(GLint), NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, UBO, 0, indexes.size() * sizeof(GLint));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, indexes.size() * sizeof(GLint), &indexes[0]);
    glCheckError();

    LOG(WARNING)<<"Uploaded "<<indexes.size() * sizeof(GLint)<<" bytes";

    /*GLuint ubo2 = glGetUniformBlockIndex(shader->Program, "TextureData");
    glUniformBlockBinding(shader->Program, ubo2, 1);
    glGenBuffers(1, &UBO2);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO2);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 1024, NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 1, UBO2, 0, 1024 * sizeof(glm::vec4));
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::vec4) * textureCoordinates.size(), textureCoordinates.data());*/

    /*
    GLuint ubo3 = glGetUniformBlockIndex(shader->Program, "TextureData2");
    glUniformBlockBinding(shader->Program, ubo3, 2);
    glGenBuffers(1, &UBO3);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO3);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::vec4) * 1024, NULL, GL_STATIC_DRAW);
    glBindBufferRange(GL_UNIFORM_BUFFER, 2, UBO3, 0, 1024 * sizeof(glm::vec4));*/

    glBindBuffer(GL_UNIFORM_BUFFER,0);
    glCheckError();
}

void Core::UpdateChunksToRender() {
    camera.Position = glm::vec3(gameState->g_PlayerX, gameState->g_PlayerY, gameState->g_PlayerZ);
    toRender.clear();
    const float ChunkDistance = 1;
    Vector playerChunk = Vector(floor(gameState->g_PlayerX / 16.0f), floor(gameState->g_PlayerY / 16.0f),
                                floor(gameState->g_PlayerZ / 16.0f));
    for (auto &it:gameState->world.m_sections) {
        Vector chunkPosition = it.first;
        Vector delta = chunkPosition - playerChunk;
        if (delta.GetDistance() > ChunkDistance)
            continue;
        toRender.push_back(chunkPosition);
    }
    LOG(INFO) << "Chunks to render: " << toRender.size();

    /*std::map<Block, int> totalBlocks;
    for (auto &section:toRender)
        for (int x = 0; x < 16; x++)
            for (int y = 0; y < 16; y++)
                for (int z = 0; z < 16; z++)
                    totalBlocks[gameState->world.m_sections.find(section)->second.GetBlock(Vector(x, y, z))]++;
    for (auto &it:totalBlocks) {
        LOG(WARNING) << it.first.id << ":" << (int) it.first.state << " = " << it.second << " ("
                     << std::bitset<13>(it.first.id) << ")";
    }*/
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
