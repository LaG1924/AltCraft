#pragma once

#include <mutex>
#include <queue>
#include <memory>

#include <glm/mat4x4.hpp>

#include "World.hpp"
#include "Window.hpp"

class Packet;
class NetworkClient;
class Entity;

class GameState {
    std::mutex packetsMutex;
    std::queue<std::shared_ptr<Packet>> packets;
public:

    GameState() = default;

    ~GameState() = default;

	void Update(float deltaTime);

    void UpdatePacket(NetworkClient *nc);

	enum Direction {
		FORWARD, BACKWARD, LEFT, RIGHT, JUMP
	};
	void HandleMovement(GameState::Direction direction, float deltaTime);
	void HandleRotation(double yaw, double pitch);
	glm::mat4 GetViewMatrix();
    Entity* player;

	World world;

	std::string g_PlayerUuid = "";
	std::string g_PlayerName = "";
	bool g_IsGameStarted = false;
	int g_PlayerEid = 0;
	int g_Gamemode = 0;
	int g_Dimension = 0;
	byte g_Difficulty = 0;
	byte g_MaxPlayers = 0;
	std::string g_LevelType = "";
	bool g_ReducedDebugInfo = false;
	Vector g_SpawnPosition;
	bool g_PlayerInvulnerable = false;
	bool g_PlayerFlying = false;
	bool g_PlayerAllowFlying = false;
	bool g_PlayerCreativeMode = false;
	float g_PlayerFlyingSpeed = 0;
	float g_PlayerFovModifier = 0;
	float g_PlayerHealth = 0;

    long long WorldAge = 0;
    long long TimeOfDay = 0;

    Window playerInventory;
    std::vector<Window> openedWindows;
};
