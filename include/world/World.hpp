#pragma once

#include <map>
#include <bitset>

#include <easylogging++.h>

#include <world/Block.hpp>
#include <world/Section.hpp>
#include <network/Packet.hpp>
#include <world/Collision.hpp>

class World {
	//utility vars
	World(const World &other);

	World &operator=(const World &other);

	//game vars
	int dimension = 0;

	//game methods
	Section ParseSection(StreamInput *data, Vector position);

public:
	World();

	~World();

	void ParseChunkData(std::shared_ptr<PacketChunkData> packet);

	std::map<Vector, Section> sections;

	bool isPlayerCollides(double X, double Y, double Z);
};