#include <iostream>
#include <bitset>
#include "World.hpp"

void World::ParseChunkData(Packet packet) {
    int chunkX = packet.GetField(0).GetInt();
    int chunkZ = packet.GetField(1).GetInt();
    bool isGroundContinuous = packet.GetField(2).GetBool();
    std::bitset<16> bitmask(packet.GetField(3).GetVarInt());
    int entities = packet.GetField(5).GetVarInt();

    size_t dataLen = packet.GetField(5).GetLength();
    byte *content = new byte[dataLen];
    byte *contentOrigPtr = content;
    packet.GetField(5).CopyToBuff(content);

    if (isGroundContinuous)
        dataLen -= 256;

    byte *biomes = content + packet.GetField(5).GetLength() - 256;
    for (int i = 0; i < 16; i++) {
        if (bitmask[i]) {
            size_t len = 0;
            m_sections[PositionI(chunkX, chunkZ, i)] = ParseSection(content, len);
            m_sectionToParse.push(m_sections.find(PositionI(chunkX, chunkZ, i)));
            m_parseSectionWaiter.notify_one();
            content += len;
        }
    }
    delete[] contentOrigPtr;
}

Section World::ParseSection(byte *data, size_t &dataLen) {
    dataLen = 0;

    Field fBitsPerBlock = FieldParser::Parse(UnsignedByte, data);
    byte bitsPerBlock = fBitsPerBlock.GetUByte();
    data += fBitsPerBlock.GetLength();
    dataLen += fBitsPerBlock.GetLength();

    Field fPaletteLength = FieldParser::Parse(VarInt, data);
    int paletteLength = fPaletteLength.GetVarInt();
    data += fPaletteLength.GetLength();
    dataLen += fPaletteLength.GetLength();

    std::vector<unsigned short> palette;
    if (paletteLength > 0) {
        for (unsigned char i = 0; i < paletteLength; i++) {
            endswap(&i);
            Field f = FieldParser::Parse(VarInt, data);
            data += f.GetLength();
            dataLen += f.GetLength();
            palette.push_back(f.GetVarInt());
            endswap(&i);
        }
    }

    Field fDataLength = FieldParser::Parse(VarInt, data);
    data += fDataLength.GetLength();
    dataLen += fDataLength.GetLength();

    int dataLength = fDataLength.GetVarInt();
    size_t dataSize = dataLength * 8;
    dataLen += dataSize;
    byte *dataBlocks = data;

    data += 2048;
    dataLen += 2048;
    byte *dataLight = data;

    byte *dataSky = nullptr;
    if (m_dimension == 0) {
        data += 2048;
        dataLen += 2048;
        dataSky = data;
    }

    return Section(dataBlocks, dataSize, dataLight, dataSky, bitsPerBlock, palette);
}

World::~World() {
    isContinue = false;
    m_parseSectionWaiter.notify_all();
    m_sectionParseThread.join();
}

void World::SectionParsingThread() {
    while (isContinue) {
        std::unique_lock<std::mutex> sectionParseLocker(m_parseSectionMutex);
        m_parseSectionWaiter.wait(sectionParseLocker);
        while (m_sectionToParse.size() == 0 && isContinue) {
            m_parseSectionWaiter.wait(sectionParseLocker);
        }
        while (m_sectionToParse.size() > 0) {
            auto it = m_sectionToParse.front();
            m_sectionToParse.pop();
            it->second.Parse();
            /*std::cout << "Parsed chunk" << it->first.GetX() << "x" << it->first.GetY() << "x" << it->first.GetZ()
                      << std::endl;*/
        }
    }
}

World::World() {
    m_sectionParseThread = std::thread(&World::SectionParsingThread, this);
}

