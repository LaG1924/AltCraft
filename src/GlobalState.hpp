#pragma once

#include <memory>
#include <thread>

class NetworkClient;
class GameState;
class Render;

struct GlobalState {
    static GameState *GetGameState();
    static Render *GetRender();
    static void Exec();
};