#pragma once

#include <SFML/Graphics.hpp>
#include "World.hpp"

class Display {
    sf::RenderWindow *window;
    World* world;
public:
    Display(int w=1280,int h=720,std::string title="AltCraft");
    ~Display();
    void SetWorld(World* wrd);
    void Update();
    bool IsClosed();
    void SetPlayerPos(float x, float z);
};

