#pragma once

#include <memory>
#include <thread>

class NetworkClient;
class GameState;
class Render;

enum class State {
    InitialLoading,
    MainMenu,
    Loading,
    Playing,
    Paused,
    Inventory,
    Chat,
};

struct GlobalState {    
    static GameState *GetGameState();
    static Render *GetRender();
    static void Exec();
    static State GetState();
    static void SetState(const State &newState);
};