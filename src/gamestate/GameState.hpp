#pragma once

#include "../world/World.hpp"
#include "../network/NetworkClient.hpp"

class GameState {
    NetworkClient *nc;
public:
    GameState(NetworkClient *NetClient);
    World world;
    void Update();

    std::string g_PlayerUuid;
    std::string g_PlayerName;
    ConnectionState m_networkState;
    bool g_IsGameStarted;
    int g_PlayerEid;
    int g_Gamemode;
    int g_Dimension;
    byte g_Difficulty;
    byte g_MaxPlayers;
    std::string g_LevelType;
    bool g_ReducedDebugInfo;
    Vector g_SpawnPosition;
    bool g_PlayerInvulnerable;
    bool g_PlayerFlying;
    bool g_PlayerAllowFlying;
    bool g_PlayerCreativeMode;
    float g_PlayerFlyingSpeed;
    float g_PlayerFovModifier;
    float g_PlayerPitch;
    float g_PlayerYaw;
    double g_PlayerX;
    double g_PlayerY;
    double g_PlayerZ;
};
