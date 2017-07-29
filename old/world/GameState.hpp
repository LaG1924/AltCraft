#pragma once

#include <nlohmann/json.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "World.hpp"
#include "../network/NetworkClient.hpp"
#include "../Vector.hpp"

class GameState {
	NetworkClient *nc;
public:
	GameState(NetworkClient *NetClient, bool &quit);

	void Update(float deltaTime);

	//Navigation
	enum Direction {
		FORWARD, BACKWARD, LEFT, RIGHT, JUMP
	};
	void HandleMovement(GameState::Direction direction, float deltaTime);
	void HandleRotation(double yaw, double pitch);
	glm::mat4 GetViewMatrix();
	void updateCameraVectors();

	float Yaw();
	float Pitch();
	void SetYaw(float yaw);
	void SetPitch(float pitch);

	glm::vec3 Position();
	void SetPosition(glm::vec3 Position);
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;

	//Everything other
	World world;
	bool &isRunning;

	std::string g_PlayerUuid;
	std::string g_PlayerName;
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
	float g_PlayerHealth;

	bool g_OnGround = true;
	double g_PlayerVelocityX = 0;
	double g_PlayerVelocityY = 0;
	double g_PlayerVelocityZ = 0;
};
