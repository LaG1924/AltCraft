#include "Stream.hpp"

#include <easylogging++.h>

#include "Socket.hpp"
#include "Utility.hpp"

const int MAX_VARINT_LENGTH = 5;

bool StreamInput::ReadBool() {
	unsigned char value;
	ReadData(&value, 1);
	return value != 0;
}

signed char StreamInput::ReadByte() {
	signed char value;
	ReadData((unsigned char *) &value, 1);
	endswap(value);
	return value;
}

unsigned char StreamInput::ReadUByte() {
	unsigned char value;
	ReadData(&value, 1);
	endswap(value);
	return value;
}

short StreamInput::ReadShort() {
	unsigned short value;
	ReadData((unsigned char *) &value, 2);
	endswap(value);
	return value;
}

unsigned short StreamInput::ReadUShort() {
	unsigned short value;
	ReadData((unsigned char *) &value, 2);
	endswap(value);
	return value;
}

int StreamInput::ReadInt() {
	int value;
	ReadData((unsigned char *) &value, 4);
	endswap(value);
	return value;
}

long long StreamInput::ReadLong() {
	long long value;
	ReadData((unsigned char *) &value, 8);
	endswap(value);
	return value;
}

float StreamInput::ReadFloat() {
	float value;
	ReadData((unsigned char *) &value, 4);
	endswap(value);
	return value;
}

double StreamInput::ReadDouble() {
	double value;
	ReadData((unsigned char *) &value, 8);
	endswap(value);
	return value;
}

std::string StreamInput::ReadString() {
	int strLength = ReadVarInt();
	unsigned char *buff = new unsigned char[strLength + 1];
	ReadData(buff, strLength);
	buff[strLength] = 0;
	std::string str((char *) buff);
	delete[] buff;
	return str;
}

Chat StreamInput::ReadChat() {
	/*std::string str, jsonStr = ReadString();
	nlohmann::json json;
	try {
		json = nlohmann::json::parse(jsonStr);
	} catch (std::exception &e) {
		LOG(WARNING) << "Chat json parsing failed: " << e.what();
		LOG(WARNING) << "Corrupted json: " << jsonStr;
		return "";
	}
	if (json.find("translate") != json.end())
		if (json["translate"].get<std::string>() == "multiplayer.disconnect.kicked")
			return "kicked by operator";
	for (auto &it:json["extra"]) {
		str += it["text"].get<std::string>();
	}*/
    Chat str(ReadString());
	return str;
}

int StreamInput::ReadVarInt() {
	unsigned char data[MAX_VARINT_LENGTH] = {0};
	size_t dataLen = 0;
	do {
		ReadData(&data[dataLen], 1);
	} while ((data[dataLen++] & 0x80) != 0);

	int readed = 0;
	int result = 0;
	char read;
	do {
		read = data[readed];
		int value = (read & 0b01111111);
		result |= (value << (7 * readed));
		readed++;
	} while ((read & 0b10000000) != 0);

	return result;
}

long long StreamInput::ReadVarLong() {
	return 0;
}

std::vector<unsigned char> StreamInput::ReadEntityMetadata() {
    LOG(FATAL) << "Reading EntityMetadata is not implemented";
	return std::vector<unsigned char>();
}

SlotDataType StreamInput::ReadSlot() {
    SlotDataType slot;
    slot.BlockId = ReadShort();

    if (slot.BlockId == -1)
        return slot;

    slot.ItemCount = ReadByte();
    slot.ItemDamage = ReadShort();

    if (ReadByte() != 0)
        throw std::runtime_error("Slot data with NBT not supported");

    return slot;
}

std::vector<unsigned char> StreamInput::ReadNbtTag() {
    LOG(FATAL) << "Reading NBT is not implemented";
	return std::vector<unsigned char>();
}

Vector StreamInput::ReadPosition() {
	unsigned long long t = ReadLong();
	int x = t >> 38;
	int y = (t >> 26) & 0xFFF;
	int z = t << 38 >> 38;
	if (x >= pow(2, 25)) {
		x -= pow(2, 26);
	}
	if (y >= pow(2, 11)) {
		y -= pow(2, 12);
	}
	if (z >= pow(2, 25)) {
		z -= pow(2, 26);
	}
	return Vector(x, y, z);
}

unsigned char StreamInput::ReadAngle() {
	return ReadUByte();
}

Uuid StreamInput::ReadUuid() {
	unsigned char buff[16];
	ReadData(buff, 16);
	endswap(buff, 16);
    return Uuid(buff,buff+16);
}

std::vector<unsigned char> StreamInput::ReadByteArray(size_t arrLength) {
	unsigned char *buffer = new unsigned char[arrLength];
	ReadData(buffer, arrLength);
	std::vector<unsigned char> ret(buffer, buffer + arrLength);
	delete[] buffer;
	return ret;

}

void StreamOutput::WriteBool(bool value) {
	unsigned char val = value ? 1 : 0;
	endswap(val);
	WriteData(&val, 1);
}

void StreamOutput::WriteByte(signed char value) {
	endswap(value);
	WriteData((unsigned char *) &value, 1);
}

void StreamOutput::WriteUByte(unsigned char value) {
	endswap(value);
	WriteData(&value, 1);
}

void StreamOutput::WriteShort(short value) {
	endswap(value);
	WriteData((unsigned char *) &value, 2);
}

void StreamOutput::WriteUShort(unsigned short value) {
	endswap(value);
	WriteData((unsigned char *) &value, 2);
}

void StreamOutput::WriteInt(int value) {
	endswap(value);
	WriteData((unsigned char *) &value, 4);
}

void StreamOutput::WriteLong(long long value) {
	endswap(value);
	WriteData((unsigned char *) &value, 8);
}

void StreamOutput::WriteFloat(float value) {
	endswap(value);
	WriteData((unsigned char *) &value, 4);
}

void StreamOutput::WriteDouble(double value) {
	endswap(value);
	WriteData((unsigned char *) &value, 8);
}

void StreamOutput::WriteString(const std::string &value) {
	WriteVarInt(value.size());
	WriteData((unsigned char *) value.data(), value.size());
}

void StreamOutput::WriteChat(const Chat &value) {
	WriteString(value.ToJson());
}

void StreamOutput::WriteVarInt(int value) {
	unsigned char buff[5];
	size_t len = 0;
	do {
		unsigned char temp = (unsigned char) (value & 0b01111111);
		value >>= 7;
		if (value != 0) {
			temp |= 0b10000000;
		}
		buff[len] = temp;
		len++;
	} while (value != 0);
	WriteData(buff, len);
}

void StreamOutput::WriteVarLong(long long value) {
	unsigned char buff[10];
	size_t len = 0;
	do {
		unsigned char temp = (unsigned char) (value & 0b01111111);
		value >>= 7;
		if (value != 0) {
			temp |= 0b10000000;
		}
		buff[len] = temp;
		len++;
	} while (value != 0);
	WriteData(buff, len);
}

void StreamOutput::WriteEntityMetadata(const std::vector<unsigned char> &value) {
	LOG(FATAL) << "Used unimplemented WriteEntityMetadata: " << value.size();
}

void StreamOutput::WriteSlot(const SlotDataType &value) {
    WriteShort(value.BlockId);
    if (value.BlockId == -1)
        return;
    WriteByte(value.ItemCount);
    WriteShort(value.ItemDamage);
    WriteByte(0);
}

void StreamOutput::WriteNbtTag(const std::vector<unsigned char> &value) {
	LOG(FATAL) << "Used unimplemented WriteNbtTag " << value.size();
}

void StreamOutput::WritePosition(const Vector &value) {
	unsigned long long pos = ((value.x & 0x3FFFFFF) << 38) | ((value.y & 0xFFF) << 26) | (value.z & 0x3FFFFFF);
	WriteLong(pos);
}

void StreamOutput::WriteAngle(unsigned char value) {
	WriteUByte(value);
}

void StreamOutput::WriteUuid(const Uuid &value) {
	WriteByteArray(value);
}

void StreamOutput::WriteByteArray(const std::vector<unsigned char> &value) {
    auto& val = const_cast<std::vector<unsigned char>&>(value);
	WriteData(val.data(), val.size());
}

void StreamBuffer::ReadData(unsigned char *buffPtr, size_t buffLen) {
	size_t bufferLengthLeft = buffer + bufferLength - bufferPtr;

    if (bufferLengthLeft < buffLen)
        throw std::runtime_error("Internal error: StreamBuffer reader out of data");
	std::memcpy(buffPtr, bufferPtr, buffLen);
	bufferPtr += buffLen;
}

void StreamBuffer::WriteData(unsigned char *buffPtr, size_t buffLen) {
	size_t bufferLengthLeft = buffer + bufferLength - bufferPtr;
	if (bufferLengthLeft < buffLen)
		throw std::runtime_error("Internal error: StreamBuffer writer out of data");
	std::memcpy(bufferPtr, buffPtr, buffLen);
	bufferPtr += buffLen;
}

StreamBuffer::StreamBuffer(unsigned char *data, size_t dataLen) {
	buffer = new unsigned char[dataLen];
	bufferPtr = buffer;
	bufferLength = dataLen;
	std::memcpy(buffer, data, dataLen);
}

StreamBuffer::StreamBuffer(size_t bufferLen) {
	buffer = new unsigned char[bufferLen];
	bufferPtr = buffer;
	bufferLength = bufferLen;
	for (unsigned char *p = buffer; p != buffer + bufferLength; ++p)
		*p = 0;
}

StreamBuffer::~StreamBuffer() {
	delete[] buffer;
}

std::vector<unsigned char> StreamBuffer::GetBuffer() {
	return std::vector<unsigned char>(buffer, buffer + bufferLength);
}

size_t StreamBuffer::GetReadedLength() {
    return bufferPtr - buffer;
}

void StreamCounter::WriteData(unsigned char *buffPtr, size_t buffLen) {
	buffPtr++;
	size += buffLen;
}

StreamCounter::StreamCounter(size_t initialSize) : size(initialSize) {

}

StreamCounter::~StreamCounter() {

}

size_t StreamCounter::GetCountedSize() {
	return size;
}

void StreamSocket::ReadData(unsigned char *buffPtr, size_t buffLen) {
	socket->Read(buffPtr, buffLen);
}

void StreamSocket::WriteData(unsigned char *buffPtr, size_t buffLen) {
	socket->Write(buffPtr, buffLen);
}

StreamSocket::StreamSocket(Socket *socketPtr) : socket(socketPtr) {

}