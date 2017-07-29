#pragma once

#include <map>
#include <bitset>

#include <easylogging++.h>

#include "Block.hpp"
#include "Section.hpp"
#include "Packet.hpp"
#include "Collision.hpp"
#include "Vector.hpp"

class World {
	std::map<Vector, Section> sections;
	std::map<Vector, std::mutex> sectionMutexes;
	int dimension = 0;

	Section ParseSection(StreamInput *data, Vector position);

	World(const World &other);
	World &operator=(const World &other);
public:
	World();

	~World();

	void ParseChunkData(std::shared_ptr<PacketChunkData> packet);

	bool isPlayerCollides(double X, double Y, double Z);

	Block &GetBlock(Vector pos);

	std::vector<Vector> GetSectionsList();

	Section &GetSection(Vector sectionPos);

	glm::vec3 Raycast(glm::vec3 position, glm::vec3 direction, float maxLength = 1000.0f, float minPrecision = 0.01f);
};