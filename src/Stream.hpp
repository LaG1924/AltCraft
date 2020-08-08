#pragma once

#include <vector>

#include <SDL2/SDL_net.h>

#include "Utility.hpp"
#include "Vector.hpp"
#include "Chat.hpp"

struct SlotDataType {
    short BlockId = -1;
    signed char ItemCount = 1;
    short ItemDamage = 0;
    //Nbt NBT;
};

class Stream {
public:
//	virtual ~Stream() {};
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
	void WriteString(const std::string &value);
	void WriteChat(const Chat &value);
	void WriteVarInt(int value);
	void WriteVarLong(long long value);
	void WriteEntityMetadata(const std::vector<unsigned char> &value);
	void WriteSlot(const SlotDataType &value);
	void WriteNbtTag(const std::vector<unsigned char> &value);
	void WritePosition(const Vector &value);
	void WriteAngle(unsigned char value);
	void WriteUuid(const Uuid &value);
	void WriteByteArray(const std::vector<unsigned char> &value);
};

class StreamBuffer : public StreamInput, public StreamOutput {
	std::vector<unsigned char> buffer;
	unsigned char *bufferPtr;

	void ReadData(unsigned char *buffPtr, size_t buffLen) override;
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;

public:
	StreamBuffer(unsigned char *data, size_t dataLen);
	StreamBuffer(size_t bufferLen);

	std::vector<unsigned char> GetBuffer();
    size_t GetReadedLength();
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
	IPaddress server;
	TCPsocket socket;

	std::vector<unsigned char> buffer;
	void ReadData(unsigned char *buffPtr, size_t buffLen) override;
	void WriteData(unsigned char *buffPtr, size_t buffLen) override;
public:
	StreamSocket(std::string &addr, Uint16 port);
	~StreamSocket() override;

	void Flush();
};
