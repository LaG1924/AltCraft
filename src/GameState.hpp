#pragma once

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "World.hpp"
#include "NetworkClient.hpp"
#include "Vector.hpp"
#include "Event.hpp"
#include "Window.hpp"

class GameState {
    std::shared_ptr<NetworkClient> nc;
public:
	GameState(std::shared_ptr<NetworkClient> networkClient);

	void Update(float deltaTime);

    void UpdatePacket();

	enum Direction {
		FORWARD, BACKWARD, LEFT, RIGHT, JUMP
	};
	void HandleMovement(GameState::Direction direction, float deltaTime);
	void HandleRotation(double yaw, double pitch);
	glm::mat4 GetViewMatrix();
    Entity* player;
	/*void updateCameraVectors();

	float Yaw();
	float Pitch();
	void SetYaw(float yaw);
	void SetPitch(float pitch);

	glm::vec3 Position();
	void SetPosition(glm::vec3 Position);
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;*/

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
	/*float g_PlayerPitch = 0;
	float g_PlayerYaw = 0;
	double g_PlayerX = 0;
	double g_PlayerY = 0;
	double g_PlayerZ = 0;*/
	float g_PlayerHealth = 0;

	/*bool g_OnGround = true;
	double g_PlayerVelocityX = 0;
	double g_PlayerVelocityY = 0;
	double g_PlayerVelocityZ = 0;*/

    long long WorldAge = 0;
    long long TimeOfDay = 0;

    std::shared_ptr<GameState> gs;

    Window playerInventory;
    std::vector<Window> openedWindows;
};
