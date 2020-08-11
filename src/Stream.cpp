#include "Stream.hpp"

#include <easylogging++.h>

#include "Utility.hpp"

#define bedoubletoh(x) be64toh(*reinterpret_cast<uint64_t*>(&value))
#define befloattoh(x) be32toh(*reinterpret_cast<uint32_t*>(&value))
#define beS64toh(x) static_cast<int64_t>(be64toh(*reinterpret_cast<uint64_t*>(&value)))
#define beS32toh(x) static_cast<int32_t>(be32toh(*reinterpret_cast<uint32_t*>(&value)))
#define beS16toh(x) static_cast<int16_t>(be16toh(*reinterpret_cast<uint16_t*>(&value)))

#define htobedouble(x) htobe64(*reinterpret_cast<uint64_t*>(&value))
#define htobefloat(x) htobe32(*reinterpret_cast<uint32_t*>(&value))
#define htobeS64(x) static_cast<int64_t>(htobe64(*reinterpret_cast<uint64_t*>(&value)))
#define htobeS32(x) static_cast<int32_t>(htobe32(*reinterpret_cast<uint32_t*>(&value)))
#define htobeS16(x) static_cast<int16_t>(htobe16(*reinterpret_cast<uint16_t*>(&value)))

const int MAX_VARINT_LENGTH = 5;

bool StreamInput::ReadBool() {
	unsigned char value;
	ReadData(&value, 1);
	return value != 0;
}

signed char StreamInput::ReadByte() {
	signed char value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 1);
	return value;
}

unsigned char StreamInput::ReadUByte() {
	unsigned char value;
	ReadData(&value, 1);
	return value;
}

short StreamInput::ReadShort() {
	unsigned short value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 2);
	return beS16toh(value);
}

unsigned short StreamInput::ReadUShort() {
	unsigned short value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 2);
	return be16toh(value);
}

int StreamInput::ReadInt() {
	int value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 4);
	return beS32toh(value);
}

long long StreamInput::ReadLong() {
	long long value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 8);
	return beS64toh(value);
}

float StreamInput::ReadFloat() {
	float value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 4);
	*reinterpret_cast<uint32_t*>(&value)=befloattoh(value);
	return value;
}

double StreamInput::ReadDouble() {
	double value;
	ReadData(reinterpret_cast<unsigned char *>(&value), 8);
	*reinterpret_cast<uint64_t*>(&value)=bedoubletoh(value);
	return value;
}

std::string StreamInput::ReadString() {
	int strLength = ReadVarInt();
	std::vector<unsigned char> buff(strLength + 1);
	ReadData(buff.data(), strLength);
	buff[strLength] = 0;
	std::string str((char *) buff.data());
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
	if (x >= 1 << 25) {
		x -= 1 << 26;
	}
	if (y >= 1 << 11) {
		y -= 1 << 12;
	}
	if (z >= 1 << 25) {
		z -= 1 << 26;
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
	std::vector<unsigned char> buffer(arrLength);
	ReadData(buffer.data(), arrLength);
	return buffer;

}

void StreamOutput::WriteBool(bool value) {
	unsigned char val = value ? 1 : 0;
	WriteData(&val, 1);
}

void StreamOutput::WriteByte(int8_t value) {
	WriteData(reinterpret_cast<unsigned char *>(&value), 1);
}

void StreamOutput::WriteUByte(uint8_t value) {
	WriteData(&value, 1);
}

void StreamOutput::WriteShort(int16_t value) {
	value=htobeS16(value);
	WriteData(reinterpret_cast<unsigned char *>(&value), 2);
}

void StreamOutput::WriteUShort(uint16_t value) {
	value=htobe16(value);
	WriteData(reinterpret_cast<unsigned char *>(&value), 2);
}

void StreamOutput::WriteInt(int32_t value) {
	value=htobeS32(value);
	WriteData(reinterpret_cast<unsigned char *>(&value), 4);
}

void StreamOutput::WriteLong(int64_t value) {
	value=htobeS64(value);
	WriteData(reinterpret_cast<unsigned char *>(&value), 8);
}

void StreamOutput::WriteFloat(float value) {
	*reinterpret_cast<uint32_t*>(&value)=htobefloat(value);
	WriteData(reinterpret_cast<unsigned char *>(&value), 4);
}

void StreamOutput::WriteDouble(double value) {
	*reinterpret_cast<uint64_t*>(&value)=htobedouble(value);
	WriteData(reinterpret_cast<unsigned char *>(&value), 8);
}

void StreamOutput::WriteString(const std::string &value) {
	WriteVarInt(value.size());
	WriteData((unsigned char *) value.data(), value.size());
}

void StreamOutput::WriteChat(const Chat &value) {
	WriteString(value.ToJson());
}

void StreamOutput::WriteVarInt(uint32_t value) {
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

void StreamOutput::WriteVarLong(uint64_t value) {
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
	size_t bufferLengthLeft = buffer.data() + buffer.size() - bufferPtr;

    if (bufferLengthLeft < buffLen)
        throw std::runtime_error("Internal error: StreamBuffer reader out of data");
	std::memcpy(buffPtr, bufferPtr, buffLen);
	bufferPtr += buffLen;
}

void StreamBuffer::WriteData(unsigned char *buffPtr, size_t buffLen) {
	size_t bufferLengthLeft = buffer.data() + buffer.size() - bufferPtr;
	if (bufferLengthLeft < buffLen)
		throw std::runtime_error("Internal error: StreamBuffer writer out of data");
	std::memcpy(bufferPtr, buffPtr, buffLen);
	bufferPtr += buffLen;
}

StreamBuffer::StreamBuffer(unsigned char *data, size_t dataLen) : buffer(data,data+dataLen) {
	bufferPtr = buffer.data();
}

StreamBuffer::StreamBuffer(size_t bufferLen) : buffer(bufferLen) {	
	bufferPtr = buffer.data();
	for (auto &it : buffer)
		it = 0;
}

std::vector<unsigned char> StreamBuffer::GetBuffer() {
	return buffer;
}

size_t StreamBuffer::GetReadedLength() {
    return bufferPtr - buffer.data();
}
