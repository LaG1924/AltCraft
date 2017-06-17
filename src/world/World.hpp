#pragma once

#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <bitset>
#include <easylogging++.h>
#include "Block.hpp"
#include "Section.hpp"
#include "../network/Packet.hpp"
#include "Collision.hpp"

class World {
	//utility vars
	World(const World &other);

	World &operator=(const World &other);

	//game vars
	int dimension = 0;

	//game methods
	Section ParseSection(StreamInput *data);

public:
	World();

	~World();

	void ParseChunkData(std::shared_ptr<PacketChunkData> packet);

	std::map<Vector, Section> sections;

	bool isPlayerCollides(double X, double Y, double Z);
};