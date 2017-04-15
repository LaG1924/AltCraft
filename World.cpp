#include <iostream>
#include <bitset>
#include "World.hpp"


Block &World::GetBlock(PositionI pos) {
    //unsigned long long pos = (unsigned long long) x << 32;
    //pos |= y;
    return m_blocks[pos];
}

void World::SetBlock(PositionI pos, Block block) {
    //unsigned long long pos = (unsigned long long) x << 32;
    //pos |= y;
    m_blocks[pos] = block;
}

void World::ParseChunkData(Packet packet) {
    int chunkX = packet.GetField(0).GetInt();
    int chunkZ = packet.GetField(1).GetInt();
    bool isGroundContinuous = packet.GetField(2).GetBool();
    std::bitset<16> bitmask(packet.GetField(3).GetVarInt());
    int entities = packet.GetField(5).GetVarInt();

    size_t dataLen = packet.GetField(5).GetLength();
    byte *content = new byte[dataLen];
    packet.GetField(5).CopyToBuff(content);

    std::cout << "Chunk " << chunkX << "x" << chunkZ << std::endl;
    std::cout << "\tGround continuous: " << (isGroundContinuous ? "true" : "false") << std::endl;
    std::cout << "\tPrimary bitmask: " << bitmask << std::endl;
    std::cout << "\tDataLen: " << dataLen << std::endl;
    std::cout << "\tEntities: " << entities << std::endl;

    if (isGroundContinuous)
        dataLen -= 256;

    byte *biomes = content + packet.GetField(5).GetLength() - 256;
    for (int i = 0; i < 1; i++) {
        if (bitmask[i]) {
            size_t len = ParseSectionData(chunkX, chunkZ, isGroundContinuous, i, content);
            //content += len;
            //std::cout << "\t Size of section is " << len << std::endl;
        }
    }
    std::cout << std::dec << std::endl;
    fflush(stdout);
}

size_t World::ParseSectionData(int chunkX, int chunkZ, bool isGroundContinous, int section, byte *data) {
    Field fBitsPerBlock = FieldParser::Parse(UnsignedByte, data);
    byte bitsPerBlock = fBitsPerBlock.GetUByte();
    data += fBitsPerBlock.GetLength();

    bool usePalette = bitsPerBlock <= 8;
    Field fPaletteLength = FieldParser::Parse(VarInt, data);
    int paletteLength = fPaletteLength.GetVarInt();
    data += fPaletteLength.GetLength();

    std::cout << "Section: " << chunkX << "x" << chunkZ << "x" << section << std::endl;
    std::cout << "\tBits per block: " << (int) bitsPerBlock << std::endl;
    std::cout << "\tPalette length: " << paletteLength << std::endl;
    std::vector<int> palette;
    std::map<unsigned char, unsigned short> pal;
    if (paletteLength > 0) {
        for (unsigned char i = 0; i < paletteLength; i++) {
            endswap(&i);
            Field f = FieldParser::Parse(VarInt, data);
            data += f.GetLength();
            palette.push_back(f.GetVarInt());
            std::cout << "\t\tPalette[" << std::bitset<8>(i) << "]: " << std::bitset<13>(palette[i]) << std::endl;
            pal[i] = f.GetVarInt();
            endswap(&i);
        }
    }

    Field fDataLength = FieldParser::Parse(VarInt, data);
    data += fDataLength.GetLength();
    int dataLength = fDataLength.GetVarInt();
    size_t dataSize = dataLength * 8;
    std::cout << "\tData length: " << dataLength << " (" << dataSize << ")" << std::endl;

    std::vector<unsigned short> blocks = ParseBlocks(data, dataLength, palette, bitsPerBlock);
    std::cout << "\tChunk content: ";
    int i = 0;
    for (int y = 0; y < SECTION_LENGTH; y++) {
        for (int z = 0; z < SECTION_HEIGHT; z++) {
            for (int x = 0; x < SECTION_WIDTH; x++) {
                int X = chunkX * SECTION_WIDTH + x;
                int Y = section * SECTION_HEIGHT + y;
                int Z = chunkZ * SECTION_LENGTH + z;
                //std::cerr<<"Block at "<<X<<" "<<Y<<" "<<Z<<std::endl;
                Block block(usePalette ? pal[blocks[i]] : blocks[i],15);
                SetBlock(PositionI(X, Z, Y), block);
                std::cout << (PositionI(X, Z, Y).GetY()) << ": "
                          << block.GetId() << "\t";
                //<< (usePalette ? pal[blocks[i]] : blocks[i]) << "\t";
                //<< std::bitset<13>(usePalette ? pal[blocks[i]] : blocks[i]) << "\t";
                i++;
            }
        }
    }
    std::cout << std::endl;
    /*for (auto it:blocks) {
        //auto m = std::bitset<4>(it).to_string();
        //std::cout<<m<<" ";
        std::cout<<std::bitset<13>(pal[it])<<" ";
        //std::cout << (m=="1000" || m =="0100" || m=="0010"?"":m+" ");
    }*/
    return (dataSize + (m_dimension == 0 ? dataSize : 0) / 2 + dataSize / 2);
}

std::vector<unsigned short>
World::ParseBlocks(byte *bytes, int dataLength, std::vector<int> palette, byte bitsPerBlock) {
    std::vector<unsigned short> blocks;
    for (int i = 0; i < blocks.size(); i++) {
        blocks[i] = 0;
    }
    byte *data = new byte[dataLength * 8];
    size_t arrLength = dataLength * 8;
    byte *ptr = data;
    for (int i = 0; i < dataLength; i++) {
        Field fData = FieldParser::Parse(Long, bytes);
        bytes += fData.GetLength();
        *reinterpret_cast<long long *>(ptr) = fData.GetLong();
        ptr += 8;
    }
    int bitPos = 0;
    unsigned short t = 0;
    for (int i = 0; i < arrLength; i++) {
        //endswap(&data[i]);
        //std::bitset<8> bitset1 = std::bitset<8>(data[i]);
        //std::cout << bitset1 << ": ";
        for (int j = 0; j < 8; j++) {
            //std::bitset<8> bitset2 = std::bitset<8>(t);
            /*std::cout << bitset2 << "_";
            std::cout << bitset1 << "\t";
            fflush(stdout);*/

            t |= (data[i] & 0x01) ? 0x80 : 0x00;
            t >>= 1;
            data[i] >>= 1;
            bitPos++;
            if (bitPos >= bitsPerBlock) {
                bitPos = 0;
                //endswap(&t);
                t >>= bitsPerBlock - 1;
                blocks.push_back(t);
                //std::cout << bitset2 << "\t";
                t = 0;
            }
            /*bitset1 = std::bitset<8>(data[i]);
            bitset2 = std::bitset<8>(t);*/
        }
    }
    //std::cout << std::endl;
    /*int bitPos = 0, bytePos = 0, pushedBlocks = 0, bits = 0;
    unsigned short t = 0;
    while (bytePos < arrLength) {
        //std::cout << std::bitset<8>(data[bytePos]) << " ";
        //int bit = data[bytePos] & 1;
        t|=(data[bytePos] & 1)<<bitPos;
        data[bytePos] = data[bytePos]>>1;


        //t = t | ((data[bytePos]<<bitPos) & 1);
        std::cout<<std::bitset<8>(t)<<"\t";
        if (bitPos >= 7) {
            bitPos = 0;
            bytePos++;
            blocks[pushedBlocks] = t;
            std::cout<<std::bitset<13>(t)<<"\t";
            t = 0;
        } else {
            bitPos++;
        }

        bits++;
        if (bits>=bitsPerBlock){
            bits=0;
            bitPos=0;
            bytePos++;
            blocks[pushedBlocks] = t;
            std::cout<<std::bitset<13>(t)<<"+\t";
            t = 0;
        } else {
            t <<= 1;
        }
        fflush(stdout);
    }*/
    //
    /*int shift = 0;
    int bx = 0;
    for (int bytmass = 0; bytmass < arrLength; bytmass++) {
        unsigned char bitinbyte = 7;
        int block = 0;
        for (int posbit = 7; posbit > 0; posbit--) {
            int b = (data[bytmass] >> posbit);
            blocks[block + shift] = blocks[block + shift] | (b << bx);
            bx++;
            if (bx > bitsPerBlock) {
                bx = 0;
                if (shift == 0) shift = 1;
                if (shift == 1) {
                    shift = 0;
                    block++;
                    if ((block * 2) > blocks.size()) {
                        std::cout<<block*2<<">"<<blocks.size()<<std::endl;
                        throw 124;
                    }
                }
            }
        }
    }*/
    //
    delete[] data;
    return blocks;
}
