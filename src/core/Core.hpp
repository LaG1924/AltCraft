#pragma once

#include <easylogging++.h>
#include <SFML/Window.hpp>
#include <GL/glew.h>
#include <iomanip>
#include <glm/gtc/type_ptr.hpp>
#include "../gamestate/GameState.hpp"
#include "../network/NetworkClient.hpp"
#include "../gui/Gui.hpp"
#include "../graphics/Camera3D.hpp"
#include "../graphics/Shader.hpp"
#include "AssetManager.hpp"

class Core {
    GameState *gameState;
    NetworkClient *client;
    sf::Window *window;
    AssetManager *assetManager;
    bool isMouseCaptured = false;
    bool isRunning = true;
    enum {
        MainMenu,
        Loading,
        Playing,
        PauseMenu,
    } currentState = Playing;
    float mouseXDelta, mouseYDelta;
    float deltaTime;
    float absTime;

    void RenderWorld(World &Target);

    void RenderGui(Gui &Target);

    void HandleMouseCapture();

    void HandleEvents();

    void InitSfml(unsigned int WinWidth, unsigned int WinHeight, std::string WinTitle);

    void InitGlew();

    void SetMouseCapture(bool IsCaptured);

    void PrepareToWorldRendering();

    void RenderFrame();

    unsigned int width();

    unsigned int height();

    void UpdateChunksToRender();

    void UpdateGameState();

    std::thread gameStateLoopThread;

    Camera3D camera;
    Shader *shader;
    GLuint VBO, VAO, VBO2;
    std::vector<Vector> toRender;

public:
    Core();

    ~Core();

    void Exec();
};
