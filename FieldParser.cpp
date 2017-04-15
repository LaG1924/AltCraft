#include "FieldParser.hpp"

Field FieldParser::Parse(FieldType type, byte *data, size_t len) {
    switch (type) {
        case VarInt:
            return ParseVarInt(data, len);
        case Boolean:
            return ParseBool(data, len);
        case String:
            return ParseString(data, len);
        case Long:
            return ParseLong(data, len);
        case Int:
            return ParseInt(data, len);
        case UnsignedByte:
            return ParseUByte(data, len);
        case Byte:
            return ParseByte(data, len);
        case Float:
            return ParseFloat(data, len);
        case Position:
            return ParsePosition(data, len);
        case Double:
            return ParseDouble(data, len);
        case ByteArray:
            return ParseByteArray(data, len);
        default:
            throw 105;
    }
}

Field FieldParser::ParseString(byte *data, size_t len) {
    Field fLen = ParseVarInt(data, 0);
    Field f;
    f.SetRaw(data, fLen.GetLength() + fLen.GetVarInt(), String);
    return f;
}

Field FieldParser::ParseBool(byte *data, size_t len) {
    Field f;
    f.SetRaw(data,1,Boolean);
    return f;
}

Field FieldParser::ParseVarInt(byte *data, size_t len) {
    if (len != 0) {
        Field f;
        f.SetRaw(data, len, VarInt);
        return f;
    }
    int val = VarIntRead(data, len);
    Field f;
    f.SetVarInt(val);
    return f;
}

Field FieldParser::ParseLong(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 8, Long);
    return f;
}

Field FieldParser::ParseInt(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 4, Int);
    return f;
}

Field FieldParser::ParseUByte(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 1, UnsignedByte);
    return f;
}

Field FieldParser::ParseByte(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 1, Byte);
    return f;
}

Field FieldParser::ParseFloat(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 4, Float);
    return f;
}

Field FieldParser::ParsePosition(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 8, Position);
    return f;
}

Field FieldParser::ParseDouble(byte *data, size_t len) {
    Field f;
    f.SetRaw(data, 8, Double);
    return f;
}

Field FieldParser::ParseByteArray(byte *data, size_t len) {
    if (len == 0)
        throw 119;
    Field f;
    f.SetRaw(data, len, Byte);
    //f.SetRaw(data, len, ByteArray);
    return f;
}
