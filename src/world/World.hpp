#pragma once

#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "Block.hpp"
#include "../packet/Packet.hpp"
#include "Section.hpp"

class World {
    //utility vars
    World(const World &other);

    World &operator=(const World &other);

    //game vars
    int m_dimension = 0;

    //game methods
    Section ParseSection(byte *data, size_t &dataLen);

public:
    World();

    ~World();

    void ParseChunkData(Packet packet);

    std::map<Vector, Section> m_sections;
};