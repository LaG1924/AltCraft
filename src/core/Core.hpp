#pragma once

#include "../gamestate/GameState.hpp"
#include "../network/NetworkClient.hpp"
#include <easylogging++.h>


class Core {
    GameState *gameState;
    NetworkClient *client;

public:
    Core();
    ~Core();
    void MainLoop();
};
