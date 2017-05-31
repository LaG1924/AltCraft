#pragma once

#include "Field.hpp"

class FieldParser {
public:
    static Field ParseVarInt(byte *data, size_t len);

    static Field ParseBool(byte *data);

    static Field ParseString(byte *data);

    static Field Parse(FieldType type, byte* data, size_t len=0);

    static Field ParseLong(byte *data);

    static Field ParseInt(byte *data);

    static Field ParseUByte(byte *data);

    static Field ParseByte(byte *data);

    static Field ParseFloat(byte *data);

    static Field ParsePosition(byte *data);

    static Field ParseDouble(byte *data);

    static Field ParseByteArray(byte *data, size_t len);
};