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
    const int ChunkDistance = 2;
    PositionI playerPos((int) playerX, (int) playerZ, (int) playerY);
    for (auto &it:world->m_sections) {
        PositionI delta = it.first-playerPos;

    }
}

void Display::MainLoop() {
    Shader shader("./shaders/simple.vs", "./shaders/simple.fs");
    Texture &texture1 = *(AssetManager::GetAsset("minecraft/textures/blocks/brick").data.texture);
    Texture &texture2 = *(AssetManager::GetAsset("minecraft/textures/blocks/beacon").data.texture);

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

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1.texture);
    glUniform1i(glGetUniformLocation(shader.Program, "texture1"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2.texture);
    glUniform1i(glGetUniformLocation(shader.Program, "texture2"), 1);

    glm::vec3 cubePositions[] = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(2.0f, 5.0f, -15.0f),
            glm::vec3(-1.5f, -2.2f, -2.5f),
            glm::vec3(-3.8f, -2.0f, -12.3f),
            glm::vec3(2.4f, -0.4f, -3.5f),
            glm::vec3(-1.7f, 3.0f, -7.5f),
            glm::vec3(1.3f, -2.0f, -2.5f),
            glm::vec3(1.5f, 2.0f, -2.5f),
            glm::vec3(1.5f, 0.2f, -1.5f),
            glm::vec3(-1.3f, 1.0f, -1.5f)
    };
    Camera3D camera;
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
        glm::mat4 projection = glm::perspective(camera.Zoom, (float) width() / (float) height(), 0.1f, 1000.0f);
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);
        for (GLuint i = 0; i < 10; i++) {
            glm::mat4 model;
            model = glm::translate(model, cubePositions[i]);

            GLfloat angle = 20.0f * (i);
            model = glm::rotate(model, glm::radians(angle * absTime), glm::vec3(1.0f, 0.3f, 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        //End of render code

        window->display();
    }

}
