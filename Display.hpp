#pragma once

#include <SFML/Graphics.hpp>
#include <thread>
#include <mutex>
#include <condition_variable>
#include "World.hpp"

class Display {
    sf::RenderWindow *window;
    std::map<PositionI, std::array<sf::Texture,16>> sectionTextures;
    World *world;
    bool isGameStarted = false;
    std::condition_variable &gameStartWaiter;
    std::mutex gameStartMutex;

    //utility methods
    void pollEvents();

    void renderWorld();

    void Update();

    sf::Texture &GetSectionTexture(PositionI pos);

    //gameState vars
    sf::Vector2f mousePos;
    int renderLayer = 70;
    //int currentId = 0;
    bool isClicked = false;
    int coeff = 10;
    double frameTime = 0;
public:
    Display(int w, int h, std::string title, World *world, std::condition_variable &gameStartWaiter);

    ~Display();

    bool IsClosed();

    void SetPlayerPos(float x, float z);

    void MainLoop();
};

