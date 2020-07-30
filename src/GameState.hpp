#pragma once

#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include <glm/mat4x4.hpp>

#include "Vector.hpp"
#include "World.hpp"
#include "Window.hpp"
#include "Platform.hpp"

class Packet;
class Entity;

struct AC_API TimeStatus {
	double interpolatedTimeOfDay = 0;
	long long worldAge = 0;
	long long timeOfDay = 0;
	bool doDaylightCycle = true;
};

struct AC_API GameStatus {
	std::string levelType;
	Vector spawnPosition;
	int gamemode = 0;
	int dimension = 0;
	unsigned char difficulty = 0;
	unsigned char maxPlayers = 0;
	bool isGameStarted = false;
	bool reducedDebugInfo = false;
};

struct AC_API PlayerStatus {
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

struct AC_API SelectionStatus {
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

	bool receivedJoinGame = false;

	bool receivedEnoughChunks = false;

	bool receivedFirstPlayerPosAndLook = false;

public:

	void Update(double deltaTime);

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

	AC_API inline Entity *GetPlayer() {
		return player;
	}

	AC_API inline const World &GetWorld() const {
		return world;
	}

	AC_API inline const TimeStatus &GetTimeStatus() const {
		return timeStatus;
	}

	AC_API inline const GameStatus &GetGameStatus() const {
		return gameStatus;
	}

	AC_API inline const PlayerStatus &GetPlayerStatus() const {
		return playerStatus;
	}

	AC_API inline const SelectionStatus &GetSelectionStatus() const {
		return selectionStatus;
	}

	AC_API inline const Window &GetInventory() const {
		return playerInventory;
	}
};
