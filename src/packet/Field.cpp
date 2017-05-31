#include "Field.hpp"

Field::Field() {
}

Field::Field(const Field &other) : m_dataLength(other.m_dataLength), m_type(other.m_type) {

    m_data = new byte[m_dataLength];
    std::copy(other.m_data,other.m_data+m_dataLength,m_data);
}

void Field::swap(Field &other) {
    std::swap(other.m_dataLength, m_dataLength);
    std::swap(other.m_data, m_data);
    std::swap(other.m_type, m_type);
}

Field &Field::operator=(Field other) {
    other.swap(*this);
    return *this;
}

Field::~Field() {
    Clear();
}

size_t Field::GetLength() {
    if (m_data != nullptr && m_dataLength == 0)
        throw 102;
    return m_dataLength;
}

void Field::Clear() {
    m_dataLength = 0;
    delete[] m_data;
    m_data = nullptr;
}

void Field::CopyToBuff(byte *ptr) {
    if (m_dataLength > 0)
        std::copy(m_data,m_data+m_dataLength,ptr);
}

void Field::SetRaw(byte *ptr, size_t len, FieldType type) {
    Clear();
    m_dataLength = len;
    m_type = type;
    m_data = new byte[m_dataLength];
    std::copy(ptr,ptr+m_dataLength,m_data);
}

int Field::GetVarInt() {

    size_t readed;
    return VarIntRead(m_data, readed);

}

void Field::SetVarInt(int value) {
    Clear();
    m_type = VarIntType;
    m_data = new byte[5];
    m_dataLength = VarIntWrite(value, m_data);
}

int Field::GetInt() {
    int value = *(reinterpret_cast<int *>(m_data));
    endswap(&value);
    return value;
}

void Field::SetInt(int value) {
    Clear();
    m_type = Int;
    m_data = new byte[4];
    m_dataLength = 4;
    int *p = reinterpret_cast<int *>(m_data);
    *p = value;
    endswap(p);
}

bool Field::GetBool() {
    return *m_data != 0x00;
}

void Field::SetBool(bool value) {
    Clear();
    m_type = Boolean;
    m_data = new byte[1];
    m_dataLength = 1;
    *m_data = value ? 0x01 : 0x00;
}

unsigned short Field::GetUShort() {
    unsigned short *p = reinterpret_cast<unsigned short *>(m_data);
    unsigned short t = *p;
    endswap(&t);
    return t;
}

void Field::SetUShort(unsigned short value) {
    Clear();
    m_type = UnsignedShort;
    m_dataLength = 2;
    m_data = new byte[2];
    unsigned short *p = reinterpret_cast<unsigned short *>(m_data);
    *p = value;
    endswap(p);
}

std::string Field::GetString() {
    Field fLen;
    byte *ptr = m_data;
    size_t l;
    int val = VarIntRead(ptr, l);
    ptr += l;
    std::string s((char *) ptr, val);
    return s;
}

void Field::SetString(std::string value) {
    Clear();
    m_type = String;
    Field fLen;
    fLen.SetVarInt(value.size());
    m_dataLength = value.size() + fLen.GetLength();
    m_data = new byte[m_dataLength];
    byte *p = m_data;
    fLen.CopyToBuff(p);
    p += fLen.GetLength();
    std::copy(value.begin(),value.end(),p);
}

long long Field::GetLong() {
    long long t = *reinterpret_cast<long long *>(m_data);
    endswap(&t);
    return t;
}

void Field::SetLong(long long value) {
    Clear();
    m_type = Long;
    m_dataLength = 8;
    m_data = new byte[m_dataLength];
    long long *p = reinterpret_cast<long long *>(m_data);
    *p = value;
    endswap(p);
}

FieldType Field::GetType() {
    return m_type;
}

byte Field::GetUByte() {
    byte t = *reinterpret_cast<byte *>(m_data);
    endswap(&t);
    return t;
}

void Field::SetUByte(byte value) {
    Clear();
    m_type = UnsignedByte;
    endswap(&value);
    m_dataLength = 1;
    m_data = new byte[m_dataLength];
    byte *p = reinterpret_cast<byte *>(m_data);
    *p = value;
}

sbyte Field::GetByte() {
    sbyte t = *reinterpret_cast<sbyte *>(m_data);
    endswap(&t);
    return t;
}

void Field::SetByte(sbyte value) {
    Clear();
    m_type = Byte8_t;
    endswap(&value);
    m_dataLength = 1;
    m_data = new byte[m_dataLength];
    sbyte *p = reinterpret_cast<sbyte *>(m_data);
    *p = value;
}

float Field::GetFloat() {
    float t = *reinterpret_cast<float *>(m_data);
    endswap(&t);
    return t;
}

void Field::SetFloat(float value) {
    Clear();
    m_type = Float;
    endswap(&value);
    m_dataLength = 4;
    m_data = new byte[m_dataLength];
    float *p = reinterpret_cast<float *>(m_data);
    *p = value;
}

Vector Field::GetPosition() {
    unsigned long long t = *reinterpret_cast<unsigned long long *>(m_data);
    endswap(&t);
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
    Vector val(x,y,z);
    return val;
}

void Field::SetPosition(Vector value) {
    Clear();
    m_type = Position;
    m_dataLength = 8;
    m_data = new byte[m_dataLength];
    unsigned long long *t = reinterpret_cast<unsigned long long *>(m_data);
    unsigned long long x = ((unsigned long long) value.GetX()) << 38;
    unsigned long long y = ((unsigned long long) value.GetY()) << 26;
    unsigned long long z = value.GetZ();
    endswap(&x);
    endswap(&z);
    endswap(&y);
    *t = x | y | z;
}

double Field::GetDouble() {
    double t = *reinterpret_cast<double *>(m_data);
    endswap(&t);
    return t;
}

void Field::SetDouble(double value) {
    Clear();
    m_type = Double;
    endswap(&value);
    m_dataLength = 8;
    m_data = new byte[m_dataLength];
    double *p = reinterpret_cast<double *>(m_data);
    *p = value;
}

size_t Field::GetFieldLength(FieldType type) {
    switch (type) {
        case UnknownType:
            return 0;
        case Boolean:
            return 1;
        case Byte8_t:
            return 1;
        case UnsignedByte:
            return 1;
        case Short:
            return 2;
        case UnsignedShort:
            return 2;
        case Int:
            return 4;
        case Long:
            return 8;
        case Float:
            return 4;
        case Double:
            return 8;
        case Position:
            return 8;
        case Angle:
            return 4;
        case Uuid:
            return 16;
        default:
            return 0;
    }
}

std::vector<Field> Field::GetArray() {
    /*std::vector<Field> vec;
    if (m_type<20){
        size_t fieldLen=GetFieldLength(m_type);
        byte* ptr = m_data;
        for (int i=0;i<m_dataLength/fieldLen;i++){
            Field f;
            f.SetRaw(ptr,fieldLen,m_type);
            vec.push_back(f);
            ptr+=fieldLen;
        }
        return vec;
    }*/
    return m_childs;
}

void Field::Attach(Field field) {
    m_childs.push_back(field);
}
