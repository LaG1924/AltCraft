#include <iomanip>
#include "Display.hpp"
#include "AssetManager.hpp"

/*GLfloat vertices[] = {
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,
        0.5f, -0.5f, -0.5f,     1.0f, 0.0f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f,     0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 0.0f,

        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f,
        0.5f, -0.5f, 0.5f,      1.0f, 0.0f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f,
        0.5f, 0.5f, 0.5f,       1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,      0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f,

        -0.5f, 0.5f, 0.5f,      1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f,     1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,      1.0f, 0.0f,

        0.5f, 0.5f, 0.5f,       1.0f, 0.0f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     0.0f, 1.0f,
        0.5f, -0.5f, 0.5f,      0.0f, 0.0f,
        0.5f, 0.5f, 0.5f,       1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,
        0.5f, -0.5f, -0.5f,     1.0f, 1.0f,
        0.5f, -0.5f, 0.5f,      1.0f, 0.0f,
        0.5f, -0.5f, 0.5f,      1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,    0.0f, 1.0f,

        -0.5f, 0.5f, -0.5f,     0.0f, 1.0f,
        0.5f, 0.5f, -0.5f,      1.0f, 1.0f,
        0.5f, 0.5f, 0.5f,       1.0f, 0.0f,
        0.5f, 0.5f, 0.5f,       1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,      0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f,     0.0f, 1.0f
};
GLuint indices[] = {
        0, 1, 2,
        0, 2, 3
};*/
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


Display::Display(unsigned int winWidth, unsigned int winHeight, const char *winTitle, World *worldPtr) : world(
        worldPtr) {
    sf::ContextSettings contextSetting;
    contextSetting.majorVersion = 3;
    contextSetting.minorVersion = 3;
    contextSetting.attributeFlags = contextSetting.Core;
    contextSetting.depthBits = 24;
    window = new sf::Window(sf::VideoMode(winWidth, winHeight), winTitle, sf::Style::Default, contextSetting);
    window->setVerticalSyncEnabled(true);
    window->setMouseCursorVisible(false);
    window->setPosition(sf::Vector2i(sf::VideoMode::getDesktopMode().width / 2 - window->getSize().x/2,
                                     sf::VideoMode::getDesktopMode().height / 2 - window->getSize().y/2));

    sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);


    //Glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        throw 3;
    }
    glViewport(0, 0, width(), height());
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
}

bool Display::IsClosed() {
    return !window->isOpen();
}

void Display::SetPlayerPos(double playerX, double playerY, double playerZ) {
    camera.Position = glm::vec3(playerX, playerY, playerZ);
    toRender.clear();
    const float ChunkDistance = 1;
    Vector playerChunk = Vector(floor(playerX / 16.0f), floor(playerY / 16.0f), floor(playerZ / 16.0f));
    for (auto &it:world->m_sections) {
        Vector chunkPosition = it.first;
        Vector delta = chunkPosition - playerChunk;
        //std::cout << delta.GetDistance() << std::endl;
        if (delta.GetDistance() > ChunkDistance)
            continue;
        /*std::cout << "Rendering " << delta.GetDistance() << " Detailed: " << delta.GetX() << " " << delta.GetZ() << " "
                  << delta.GetY() << std::endl <<
                  "\t" << chunkPosition.GetX() << " " << chunkPosition.GetZ() << " "
                  << chunkPosition.GetY() << std::endl;*/
        toRender.push_back(chunkPosition);
    }
    std::cout << "Chunks to render: " << toRender.size() << std::endl;
}

void Display::MainLoop() {
    Shader shader("./shaders/simple.vs", "./shaders/simple.fs");

    GLuint VBO, VAO, VBO2;
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

    shader.Use();

    bool captureMouse = true;

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    bool isRunning = true;
    while (isRunning) {
        static sf::Clock clock, clock1;
        float deltaTime = clock.getElapsedTime().asSeconds();
        float absTime = clock1.getElapsedTime().asSeconds();
        clock.restart();
        sf::Event event;
        /*static sf::Clock clock2;
        if (clock2.getElapsedTime().asSeconds() > 5.0f) {
            clock2.restart();
            SetPlayerPos(camera.Position.x, camera.Position.y, camera.Position.z);
        }*/
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
                            captureMouse = !captureMouse;
                            window->setMouseCursorVisible(!captureMouse);
                            sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2),
                                                   *window);
                            break;
                        case sf::Keyboard::R:
                            shader.Reload();
                            break;
                        default:
                            break;
                    }
                case sf::Event::MouseWheelScrolled:
                    camera.ProcessMouseScroll(event.mouseWheelScroll.delta);
                    break;
                default:
                    break;
            }
        }
        std::ostringstream toWindow;
        glm::highp_vec3 cameraPosition(camera.Position);
        toWindow << std::setprecision(2) << std::fixed << "Pos: " << cameraPosition.x << ", " << cameraPosition.y
                 << ", " << cameraPosition.z << "; ";
        toWindow << "FPS: " << (1.0f / deltaTime) << " ";
        window->setTitle(toWindow.str());
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
        if (captureMouse) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
            sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
            sf::Mouse::setPosition(center, *window);
            int deltaX = (mousePos - center).x, deltaY = (center - mousePos).y;
            camera.ProcessMouseMovement(deltaX, deltaY);
        }

        //Render code
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shader.Use();

        GLint modelLoc = glGetUniformLocation(shader.Program, "model");
        GLint projectionLoc = glGetUniformLocation(shader.Program, "projection");
        GLint viewLoc = glGetUniformLocation(shader.Program, "view");
        GLint blockLoc = glGetUniformLocation(shader.Program, "block");
        GLint timeLoc = glGetUniformLocation(shader.Program, "time");
        glm::mat4 projection = glm::perspective(camera.Zoom, (float) width() / (float) height(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniform1f(timeLoc, absTime);

        glBindVertexArray(VAO);

        for (auto &sectionPos:toRender) {
            Section &section = world->m_sections.find(sectionPos)->second;
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

                        std::string textureName = AssetManager::GetAssetNameByBlockId(block.id);
                        if (textureName.find("air") != std::string::npos)
                            continue;
                        Texture &texture1 = *(AssetManager::GetAsset(textureName).data.texture);

                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, texture1.texture);
                        glUniform1i(glGetUniformLocation(shader.Program, "blockTexture"), 0);

                        glDrawArrays(GL_TRIANGLES, 0, 36);
                    }
                }
            }
        }
        glBindVertexArray(0);

        //End of render code

        window->display();
    }

}
