#include "Stream.hpp"

#include <easylogging++.h>

#include "Utility.hpp"

#define bedoubletoh(x) be64toh(*reinterpret_cast<uint64_t*>(&x))
#define befloattoh(x) be32toh(*reinterpret_cast<uint32_t*>(&x))
#define beS64toh(x) be64toh(*reinterpret_cast<uint64_t*>(&x))
#define beS32toh(x) be32toh(*reinterpret_cast<uint32_t*>(&x))
#define beS16toh(x) be16toh(*reinterpret_cast<uint16_t*>(&x))

#define htobedouble(x) htobe64(*reinterpret_cast<uint64_t*>(&x))
#define htobefloat(x) htobe32(*reinterpret_cast<uint32_t*>(&x))
#define htobeS64(x) htobe64(*reinterpret_cast<uint64_t*>(&x))
#define htobeS32(x) htobe32(*reinterpret_cast<uint32_t*>(&x))
#define htobeS16(x) htobe16(*reinterpret_cast<uint16_t*>(&x))

const int MAX_VARINT_LENGTH = 5;

bool StreamInput::ReadBool() {
	assert(position+1 <= this->size);
	return buffer[position++];
}

signed char StreamInput::ReadByte() {
	assert(position+1 <= this->size);
	return *reinterpret_cast<int8_t*>(&buffer[position++]);
}

unsigned char StreamInput::ReadUByte() {
	assert(position+1 <= this->size);
	return buffer[position++];
}

short StreamInput::ReadShort() {
	int16_t value;
	assert(position+2 <= this->size);
	*reinterpret_cast<uint16_t*>(&value) = beS16toh(buffer[position]);
	position+=2;
	return value;
}

unsigned short StreamInput::ReadUShort() {
	unsigned short value;
	assert(position+2 <= this->size);
	value = beS16toh(buffer[position]);
	position+=2;
	return value;
}

int32_t StreamInput::ReadInt() {
	int32_t value;
	assert(position+4 <= this->size);
	*reinterpret_cast<uint32_t*>(&value) = beS32toh(buffer[position]);
	position+=4;
	return value;
}

long long StreamInput::ReadLong() {
	int64_t value;
	assert(position+8 <= this->size);
	*reinterpret_cast<uint64_t*>(&value) = beS64toh(buffer[position]);
	position+=8;
	return value;
}

float StreamInput::ReadFloat() {
	float value;
	assert(position+4 <= this->size);
	*reinterpret_cast<uint32_t*>(&value)=befloattoh(buffer[position]);
	position+=4;
	return value;
}

double StreamInput::ReadDouble() {
	double value;
	assert(position+8 <= this->size);
	*reinterpret_cast<uint64_t*>(&value)=bedoubletoh(buffer[position]);
	position+=8;
	return value;
}

std::string StreamInput::ReadString() {
	int strLength = ReadVarInt();
	assert(position+strLength <= this->size);
	std::string str(buffer+position, buffer+position+strLength);
	position+=strLength;
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
	int readed = 0;
	int result = 0;
	char read;
	do {
		assert(readed<5);
		read = buffer[readed+position];
		int value = (read & 0b01111111);
		result |= (value << (7 * readed));
		readed++;
	} while ((read & 0b10000000) != 0);

	assert(position+readed <= this->size);
	position+=readed;

	return result;
}

long long StreamInput::ReadVarLong() {
	//WARNING VarLong not implemented
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
	assert(position+8 <= this->size);
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
	assert(position+16 <= this->size);
	memcpy(buff, buffer+position, 16);
	endswap(buff, 16);
	position+=16;
    return Uuid(buff,buff+16);
}

std::vector<uint8_t> StreamInput::ReadByteArray(size_t arrLength) {
	assert(position+arrLength <= this->size);
	std::vector<uint8_t> buff(buffer+position, buffer+position+arrLength);
	position+=arrLength;
	return buff;

}

void StreamOutput::WriteData(uint8_t *ptr, size_t size){
	assert(position+size <= this->size);
	memcpy(buffer+position, ptr, size);
	position+=size;
}

void StreamOutput::WriteBool(bool value) {
	assert(position+1 <= this->size);
	uint8_t val = value ? 1 : 0;
	buffer[position++]=val;
}

void StreamOutput::WriteByte(int8_t value) {
	assert(position+sizeof(value) <= this->size);
	buffer[position++]=*reinterpret_cast<uint8_t*>(&value);
}

void StreamOutput::WriteUByte(uint8_t value) {
	assert(position+sizeof(value) <= this->size);
	buffer[position++]=value;
}

void StreamOutput::WriteShort(int16_t value) {
	assert(position+sizeof(value) <= this->size);
	*reinterpret_cast<uint16_t*>(&buffer[position]) = htobeS16(value);
	position+=2;
}

void StreamOutput::WriteUShort(uint16_t value) {
	assert(position+sizeof(value) <= this->size);
	*reinterpret_cast<uint16_t*>(&buffer[position]) = htobe16(value);
	position+=2;
}

void StreamOutput::WriteInt(int32_t value) {
	assert(position+sizeof(value) <= this->size);
	*reinterpret_cast<uint32_t*>(&buffer[position]) = htobeS32(value);
	position+=4;
}

void StreamOutput::WriteLong(int64_t value) {
	assert(position+sizeof(value) <= this->size);
	*reinterpret_cast<uint64_t*>(&buffer[position]) = htobeS64(value);
	position+=8;
}

void StreamOutput::WriteFloat(float value) {
	assert(position+4 <= this->size);
	*reinterpret_cast<uint32_t*>(&buffer[position]) = htobefloat(value);
	position+=4;
}

void StreamOutput::WriteDouble(double value) {
	assert(position+8 <= this->size);
	*reinterpret_cast<uint64_t*>(&buffer[position]) = htobedouble(value);
	position+=8;
}

void StreamOutput::WriteString(const std::string &value) {
	assert(position+value.size()+VarIntLen(value.size()) <= this->size);
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
	assert(position+len <= this->size);
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
	assert(position+len <= this->size);
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

StreamROBuffer::StreamROBuffer(unsigned char *data, size_t size) : bufferVector(data,data+size) {
	this->size = size;
	this->position = 0;
	buffer = bufferVector.data();
}
StreamROBuffer::StreamROBuffer(size_t size) : bufferVector(size) {
	this->size = size;
	this->position = 0;
	buffer = bufferVector.data();
}

size_t StreamROBuffer::GetReadedLength() {
	return position;
}


StreamWOBuffer::StreamWOBuffer(size_t size, size_t offset) : bufferVector(size) {
	this->size = size;
	this->position = offset;
	buffer = bufferVector.data();
}

size_t StreamWOBuffer::GetReadedLength() {
	return position;
}
