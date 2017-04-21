#pragma once

#include "PositionI.hpp"
#include "NetworkClient.hpp"
#include "World.hpp"
#include "Display.hpp"

class Game {
public:
    Game();

    ~Game();

    void Exec();

private:
    //utility variables
    NetworkClient *m_nc;
    std::thread m_ncThread;
    bool m_exit = false;
    ConnectionState m_networkState = ConnectionState::Handshaking;
    Display *m_display;
    std::thread m_gameThread;

    //utility methods
    void ParsePackets();
    void MainLoop();

    //GameState update - condVars
    std::condition_variable gameStartWaiter;

    //game state variables
    World m_world;

    std::string g_PlayerUuid;
    std::string g_PlayerName;
    int g_PlayerEid;
    byte g_Gamemode;
    byte g_Difficulty;
    int g_Dimension;
    byte g_MaxPlayers;
    std::string g_LevelType;
    bool g_ReducedDebugInfo;
    PositionI g_SpawnPosition;
    bool g_PlayerInvulnerable;
    bool g_PlayerFlying;
    bool g_PlayerAllowFlying;
    bool g_PlayerCreativeMode;
    int g_PlayerFlyingSpeed;
    int g_PlayerFovModifier;
    bool g_IsGameStarted = false;
    float g_PlayerPitch;
    float g_PlayerYaw;
    double g_PlayerX;
    double g_PlayerY;
    double g_PlayerZ;
};

