#pragma once

#include <algorithm>
#include <string>
#include <stdexcept>
#include <vector>
#include <cstring>

#include <nlohmann/json.hpp>
#include <easylogging++.h>

#include "Socket.hpp"
#include "Vector.hpp"
#include "Utility.hpp"

struct SlotData {
    short BlockId = -1;
    signed char ItemCount = 1;
    short ItemDamage = 0;
    //Nbt NBT;
};

class Stream {
public:
	virtual ~Stream() {};
};

class StreamInput : Stream {
	virtual void ReadData(unsigned char *buffPtr, size_t buffLen) = 0;
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
	std::string ReadChat();
	int ReadVarInt();
	long long ReadVarLong();
	std::vector<unsigned char> ReadEntityMetadata();
	SlotData ReadSlot();
	std::vector<unsigned char> ReadNbtTag();
	Vector ReadPosition();
	unsigned char ReadAngle();
	Uuid ReadUuid();
	std::vector<unsigned char> ReadByteArray(size_t arrLength);
};

class StreamOutput : Stream {
	virtual void WriteData(unsigned char *buffPtr, size_t buffLen) = 0;
public:
	virtual ~StreamOutput() = default;
	void WriteBool(bool value);
	void WriteByte(signed char value);
	void WriteUByte(unsigned char value);
	void WriteShort(short value);
	void WriteUShort(unsigned short value);
	void WriteInt(int value);
	void WriteLong(long long value);
	void WriteFloat(float value);
	void WriteDouble(double value);
	void WriteString(std::string value);
	void WriteChat(std::string value);
	void WriteVarInt(int value);
	void WriteVarLong(long long value);
	void WriteEntityMetadata(std::vector<unsigned char> value);
	void WriteSlot(SlotData value);
	void WriteNbtTag(std::vector<unsigned char> value);
	void WritePosition(Vector value);
	void WriteAngle(unsigned char value);
	void WriteUuid(Uuid value);
	void WriteByteArray(std::vector<unsigned char> value);
};

class StreamBuffer : public StreamInput, public StreamOutput {
	unsigned char *buffer;
	unsigned char *bufferPtr;
	size_t bufferLength;

	void ReadData(unsigned char *buffPtr, size_t buffLen) override;
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;

public:
	StreamBuffer(unsigned char *data, size_t dataLen);
	StreamBuffer(size_t bufferLen);
	~StreamBuffer();

	std::vector<unsigned char> GetBuffer();
};

class StreamCounter : public StreamOutput {
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;

	size_t size;
public:
	StreamCounter(size_t initialSize = 0);
	~StreamCounter();

	size_t GetCountedSize();
};

class StreamSocket : public StreamInput, public StreamOutput {
	Socket *socket;
	void ReadData(unsigned char *buffPtr, size_t buffLen) override;
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;
public:
	StreamSocket(Socket *socketPtr);
	~StreamSocket() = default;
};