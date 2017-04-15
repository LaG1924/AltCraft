#pragma once

#include "Field.hpp"

class FieldParser {
public:
    static Field ParseVarInt(byte *data, size_t len);

    static Field ParseBool(byte *data, size_t len);

    static Field ParseString(byte *data, size_t len);

    static Field Parse(FieldType type, byte* data, size_t len=0);

    static Field ParseLong(byte *data, size_t len);

    static Field ParseInt(byte *data, size_t len);

    static Field ParseUByte(byte *data, size_t len);

    static Field ParseByte(byte *data, size_t len);

    static Field ParseFloat(byte *data, size_t len);

    static Field ParsePosition(byte *data, size_t len);

    static Field ParseDouble(byte *data, size_t len);

    static Field ParseByteArray(byte *data, size_t len);
};