#pragma once

#include <vector>

#include "Utility.hpp"
#include "Vector.hpp"
#include "Chat.hpp"

struct SlotDataType {
    short BlockId = -1;
    signed char ItemCount = 1;
    short ItemDamage = 0;
    //Nbt NBT;
};

struct Stream {
        uint8_t *buffer;
        size_t position, size;
};

//class Stream {
//protected:
//	uint8_t *buffer;
//	size_t position, size;
//public:
////	virtual ~Stream() {};
//        uint8_t* GetBuffer() const noexcept;
//        size_t GetSize() const noexcept;
//};

class StreamInput : public Stream {
public:
	virtual ~StreamInput() = default;
	bool ReadBool();
	signed char ReadByte();
	unsigned char ReadUByte();
	short ReadShort();
	unsigned short ReadUShort();
	int ReadInt();
	long long ReadLong();
	float ReadFloat();
	double ReadDouble();
	std::string ReadString();
	Chat ReadChat();
	int ReadVarInt();
	long long ReadVarLong();
	std::vector<unsigned char> ReadEntityMetadata();
	SlotDataType ReadSlot();
	std::vector<unsigned char> ReadNbtTag();
	Vector ReadPosition();
	unsigned char ReadAngle();
	Uuid ReadUuid();
	std::vector<unsigned char> ReadByteArray(size_t arrLength);
};

class StreamOutput : public Stream {
public:
	virtual ~StreamOutput() = default;
	void WriteData(uint8_t *ptr, size_t size);
	void WriteBool(bool value);
	void WriteByte(int8_t value);
	void WriteUByte(uint8_t value);
	void WriteShort(int16_t value);
	void WriteUShort(uint16_t value);
	void WriteInt(int32_t value);
	void WriteLong(int64_t value);
	void WriteFloat(float value);
	void WriteDouble(double value);
	void WriteString(const std::string &value);
	void WriteChat(const Chat &value);
	void WriteVarInt(uint32_t value);
	void WriteVarLong(uint64_t value);
	void WriteEntityMetadata(const std::vector<unsigned char> &value);
	void WriteSlot(const SlotDataType &value);
	void WriteNbtTag(const std::vector<unsigned char> &value);
	void WritePosition(const Vector &value);
	void WriteAngle(unsigned char value);
	void WriteUuid(const Uuid &value);
	void WriteByteArray(const std::vector<unsigned char> &value);
};

class StreamROBuffer : public StreamInput {
	std::vector<unsigned char> bufferVector;
public:
	StreamROBuffer(unsigned char *data, size_t size);
	StreamROBuffer(size_t size);

	size_t GetReadedLength();
};

class StreamWOBuffer : public StreamOutput {
	std::vector<uint8_t> bufferVector;
public:
	StreamWOBuffer(size_t size, size_t offset = 0);

	size_t GetReadedLength();
};
