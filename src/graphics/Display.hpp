#pragma once

#include <SFML/Window.hpp>
#include "../world/World.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.hpp"
#include "Texture.hpp"
#include "Camera3D.hpp"

class Display {
    sf::Window *window;
    World* world;
    std::vector<Vector> toRender;
    Camera3D camera;
public:
    Display(unsigned int winWidth, unsigned int winHeight, const char winTitle[9], World *worldPtr);

    bool IsClosed();

    void SetPlayerPos(double playerX, double playerY, double playerZ);

    void MainLoop();

    unsigned int width() {
        return window->getSize().x;
    }

    unsigned int height() {
        return window->getSize().y;
    }
};