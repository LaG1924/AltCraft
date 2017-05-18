#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "../utility/utility.h"
#include "../utility/Vector.hpp"

typedef unsigned char byte;
typedef signed char sbyte;

enum FieldType {
    Unknown = 0,
    Boolean,        //Bool
    Byte8_t,           //int8_t
    UnsignedByte,   //uint8_t
    Short,          //int16_t
    UnsignedShort,  //uint16_t
    Int,            //int32_t
    Long,           //int64_t
    Float,          //float
    Double,         //double
    Position,       //PositionI
    Angle,          //uint8_t
    Uuid,           //byte* (2 bytes)
    //Unknown-length data

    String = 100,   //std::string
    Chat,           //std::string
    VarInt,         //int32_t
    VarLong,        //int64_t
    ChunkSection,   //byte*
    EntityMetadata, //byte*
    Slot,           //byte*
    NbtTag,         //byte*
    ByteArray,      //byte*
};

class Field {
public:
    Field();

    Field(const Field &other);

    void swap(Field &other);

    Field &operator=(Field other);

    ~Field();

    size_t GetLength();

    void Clear();

    void CopyToBuff(byte *ptr);

    void SetRaw(byte *ptr, size_t len = 0, FieldType type = Unknown);

    FieldType GetType();

    void Attach(Field field);

    static size_t GetFieldLength(FieldType type);

    //Cpp-types setters/getters for binary content of MC's data types

    int GetVarInt();

    void SetVarInt(int value);

    int GetInt();

    void SetInt(int value);

    bool GetBool();

    void SetBool(bool value);

    unsigned short GetUShort();

    void SetUShort(unsigned short value);

    std::string GetString();

    void SetString(std::string value);

    long long GetLong();

    void SetLong(long long value);

    byte GetUByte();

    void SetUByte(byte value);

    sbyte GetByte();

    void SetByte(sbyte value);

    float GetFloat();

    void SetFloat(float value);

    Vector GetPosition();

    void SetPosition(Vector value);

    double GetDouble();

    void SetDouble(double value);

    std::vector<Field> GetArray();

private:
    size_t m_dataLength = 0;
    byte *m_data = nullptr;
    FieldType m_type = Unknown;
    std::vector<Field> m_childs;
};
