#include "Core.hpp"

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
    InitSfml(1280, 720, "AltCraft");
    InitGlew();
    PrepareToWorldRendering();
    client = new NetworkClient("127.0.0.1", 25565, "HelloOne");
    gameState = new GameState(client);
    std::thread loop = std::thread(&Core::UpdateGameState,this);
    std::swap(loop,gameStateLoopThread);
    assetManager = new AssetManager;
    LOG(INFO) << "Core is initialized";
}

Core::~Core() {
    LOG(INFO) << "Core stopping...";
    delete shader;
    delete client;
    delete gameState;
    LOG(INFO) << "Core is stopped";
}

void Core::Exec() {
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
            RenderWorld(gameState->world);
            //RenderGui(HUD);
            break;
        case PauseMenu:
            RenderWorld(gameState->world);
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
    window->setVerticalSyncEnabled(true);
    window->setPosition(sf::Vector2i(sf::VideoMode::getDesktopMode().width / 2 - window->getSize().x / 2,
                                     sf::VideoMode::getDesktopMode().height / 2 - window->getSize().y / 2));

    SetMouseCapture(false);
}

void Core::InitGlew() {
    LOG(INFO) << "Initializing GLEW";
    glewExperimental = GL_TRUE;
    GLenum glewStatus = glewInit();
    if (glewStatus != GLEW_OK) {
        LOG(FATAL) << "Failed to initialize GLEW: " << glewGetErrorString(glewStatus);
    }
    glViewport(0, 0, width(), height());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
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
                isRunning = false;
                break;
            case sf::Event::Resized:
                glViewport(0, 0, width(), height());
                break;
            case sf::Event::KeyPressed:
                switch (event.key.code) {
                    case sf::Keyboard::Escape:
                        isRunning = false;
                        break;
                    case sf::Keyboard::T:
                        SetMouseCapture(!isMouseCaptured);
                        break;
                    default:
                        break;
                }
            case sf::Event::MouseWheelScrolled:
                //camera.ProcessMouseScroll(event.mouseWheelScroll.delta);
                break;
            default:
                break;
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
}

void Core::HandleMouseCapture() {
    sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
    sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
    sf::Mouse::setPosition(center, *window);
    mouseXDelta = (mousePos - center).x, mouseYDelta = (center - mousePos).y;
    camera.ProcessMouseMovement(mouseXDelta, mouseYDelta);
}

void Core::RenderGui(Gui &Target) {

}

void Core::RenderWorld(World &Target) {
    shader->Use();

    GLint modelLoc = glGetUniformLocation(shader->Program, "model");
    GLint projectionLoc = glGetUniformLocation(shader->Program, "projection");
    GLint viewLoc = glGetUniformLocation(shader->Program, "view");
    GLint blockLoc = glGetUniformLocation(shader->Program, "block");
    GLint timeLoc = glGetUniformLocation(shader->Program, "time");
    glm::mat4 projection = glm::perspective(camera.Zoom, (float) width() / (float) height(), 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniform1f(timeLoc, absTime);

    glBindVertexArray(VAO);

    for (auto &sectionPos:toRender) {
        Section &section = gameState->world.m_sections.find(sectionPos)->second;
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 16; z++) {
                for (int x = 0; x < 16; x++) {
                    glm::mat4 model;
                    model = glm::translate(model,
                                           glm::vec3(sectionPos.GetX() * 16, sectionPos.GetY() * 16,
                                                     sectionPos.GetZ() * 16));
                    model = glm::translate(model, glm::vec3(x, y, z));

                    Block block = section.GetBlock(Vector(x, y, z));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
                    glUniform1i(blockLoc, block.id);

                    glActiveTexture(GL_TEXTURE0);
                    //glBindTexture(GL_TEXTURE_2D, texture1.texture);
                    glUniform1i(glGetUniformLocation(shader->Program, "blockTexture"), 0);

                    glDrawArrays(GL_TRIANGLES, 0, 36);
                }
            }
        }
    }
    glBindVertexArray(0);
}

void Core::SetMouseCapture(bool IsCaptured) {
    window->setMouseCursorVisible(!isMouseCaptured);
    sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);
    isMouseCaptured = IsCaptured;
    window->setMouseCursorVisible(!IsCaptured);
}

void Core::PrepareToWorldRendering() {
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

    shader = new Shader("./shaders/simple.vs", "./shaders/simple.fs");
    shader->Use();
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
}

void Core::UpdateGameState() {
    while (gameState && client){
        gameState->Update();
    }
}
