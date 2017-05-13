#include <iomanip>
#include "Display.hpp"
#include "AssetManager.hpp"

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
    sf::Mouse::setPosition(sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2), *window);

    //Glew
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        throw 3;
    }
    glViewport(0, 0, width(), height());
    glEnable(GL_DEPTH_TEST);
}

bool Display::IsClosed() {
    return !window->isOpen();
}

void Display::SetPlayerPos(double playerX, double playerY, double playerZ) {
    camera.Position = glm::vec3(playerX, playerY, playerZ);
    const int ChunkDistance = 1;
    PositionI playerChunk = PositionI((int) playerX / 16, (int) playerZ / 16, (int) playerY / 16);
    /*std::cout << "Player chunk position: " << playerChunk.GetX() << " "
              << playerChunk.GetZ() << " " << playerChunk.GetY() << std::endl;*/
    for (auto &it:world->m_sections) {
        PositionI chunkPosition = it.first;
        PositionI delta = chunkPosition - playerChunk;
        if (delta.GetDistance() > ChunkDistance)
            continue;
        /*std::cout << "Rendering " << delta.GetDistance() << " Detailed: " << delta.GetX() << " " << delta.GetZ() << " "
                  << delta.GetY() << std::endl <<
                  "\t" << chunkPosition.GetX() << " " << chunkPosition.GetZ() << " "
                  << chunkPosition.GetY() << std::endl;*/
        toRender.push_back(it.first);
    }
    std::cout << "Chunks to render: " << toRender.size() << std::endl;
}

void Display::MainLoop() {
    Shader shader("./shaders/simple.vs", "./shaders/simple.fs");

    GLfloat vertices[] = {
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,

            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f
    };
    GLuint indices[] = {
            0, 1, 2,
            0, 2, 3
    };
    GLuint VBO, VAO, EBO;
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    {
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(2);
    }
    glBindVertexArray(0);

    shader.Use();

    bool captureMouse = true;

    bool isRunning = true;
    while (isRunning) {
        static sf::Clock clock, clock1;
        float deltaTime = clock.getElapsedTime().asSeconds();
        float absTime = clock1.getElapsedTime().asSeconds();
        clock.restart();
        sf::Event event;
        while (window->pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed:
                    window->close();
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
        if (captureMouse) {
            sf::Vector2i mousePos = sf::Mouse::getPosition(*window);
            sf::Vector2i center = sf::Vector2i(window->getSize().x / 2, window->getSize().y / 2);
            sf::Mouse::setPosition(center, *window);
            int deltaX = (mousePos - center).x, deltaY = (center - mousePos).y;
            camera.ProcessMouseMovement(deltaX, deltaY);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
            camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
            camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
            camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
            camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);


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
        /*for (GLuint i = 0; i < 10; i++) {
            glm::mat4 model;
            glm::vec3 cubePositions[] = {
                    glm::vec3(0, 0, 0),
                    glm::vec3(0, 0, 1),
                    glm::vec3(0, 0, 2),
                    glm::vec3(1, 0, 0),
                    glm::vec3(1, 0, 1),
                    glm::vec3(1, 0, 2),
                    glm::vec3(2, 0, 0),
                    glm::vec3(2, 0, 1),
                    glm::vec3(2, 0, 2),
                    glm::vec3(3, 0, 3),
            };
            if (toRender.size()<1)
                continue;
            model = glm::translate(model,
                                   glm::vec3(toRender[0].GetX() * 16, toRender[0].GetZ() * 16,
                                             toRender[0].GetY() * 16));
            model = glm::translate(model, cubePositions[i]);

            GLfloat angle = 20.0f * (i);
            //model = glm::rotate(model, glm::radians(angle * absTime), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }*/

        for (auto &sectionPos:toRender) {
            Section &section = world->m_sections[sectionPos];
            for (int y = 0; y < 16; y++) {
                for (int z = 0; z < 16; z++) {
                    for (int x = 0; x < 16; x++) {
                        glm::mat4 model;
                        model = glm::translate(model,
                                               glm::vec3(sectionPos.GetX() * 16, sectionPos.GetY() * 16,
                                                         sectionPos.GetZ() * 16));
                        model = glm::translate(model, glm::vec3(x, y, z));

                        Block block = section.GetBlock(PositionI(x, z, y));
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
