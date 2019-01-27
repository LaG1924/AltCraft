#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include <glm/mat4x4.hpp>

#include "Vector.hpp"
#include "World.hpp"
#include "Window.hpp"

class Packet;
class Entity;

struct TimeStatus {
	double interpolatedTimeOfDay = 0;
	long long worldAge = 0;
	long long timeOfDay = 0;
	bool doDaylightCycle = true;
};

struct GameStatus {
	std::string levelType;
	Vector spawnPosition;
	int gamemode = 0;
	int dimension = 0;
	unsigned char difficulty = 0;
	unsigned char maxPlayers = 0;
	bool isGameStarted = false;
	bool reducedDebugInfo = false;	
};

struct PlayerStatus {
	std::string uid;
	std::string name;
	float flyingSpeed = 0;
	float fovModifier = 0;
	float health = 0;
	int eid = 0;
	bool invulnerable = false;
	bool flying = false;
	bool allowFlying = false;
	bool creativeMode = false;
};

struct SelectionStatus {
	VectorF raycastHit;
	Vector selectedBlock;
	float distanceToSelectedBlock;
	bool isBlockSelected;
};

class GameState {
	Entity* player = nullptr;

	World world;
	
	TimeStatus timeStatus;

	GameStatus gameStatus;

	PlayerStatus playerStatus;
	
	SelectionStatus selectionStatus;
	
	Window playerInventory;

	std::vector<Window> openedWindows;

	std::mutex accessMutex;
public:

    void Update(float deltaTime);

    void UpdatePacket(std::shared_ptr<Packet> ptr);

    void StartDigging();

    void FinishDigging();

    void CancelDigging();

    void PlaceBlock();

	enum MoveType {
		FORWARD, BACKWARD, LEFT, RIGHT, JUMP
	};

    void HandleMovement(GameState::MoveType direction, float deltaTime);

    void HandleRotation(double yaw, double pitch);

    glm::mat4 GetViewMatrix();

	inline Entity *GetPlayer() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return player;
	}

	inline World GetWorld() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return world;
	}

	inline TimeStatus GetTimeStatus() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return timeStatus;
	}

	inline GameStatus GetGameStatus() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return gameStatus;
	}

	inline PlayerStatus GetPlayerStatus() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return playerStatus;
	}

	inline SelectionStatus GetSelectionStatus() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return selectionStatus;
	}

	inline Window &GetInventory() {
		std::lock_guard<std::mutex> guard(accessMutex);
		return playerInventory;
	}
};
