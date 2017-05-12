#pragma once

#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "Block.hpp"
#include "Packet.hpp"
#include "Section.hpp"

class World {
    //utility vars
    World(const World& other);
    World&operator=(const World &other);
    bool isContinue=true;
    std::mutex m_parseSectionMutex;
    std::condition_variable m_parseSectionWaiter;
    std::thread m_sectionParseThread;
    std::queue<std::map<PositionI,Section>::iterator> m_sectionToParse;
    //utility methods
    void SectionParsingThread();
    //game vars
    int m_dimension = 0;
    //game methods
    Section ParseSection(byte *data, size_t &dataLen);
public:
    World();
    ~World();
    void ParseChunkData(Packet packet);
    std::map<PositionI, Section> m_sections;
};