#include "Packet.hpp"

Packet::Packet(int id) {
    Field fLen;
    fLen.SetVarInt(0);
    m_fields.push_back(fLen);
    Field fId;
    fId.SetVarInt(id);
    m_fields.push_back(fId);
}

Packet Packet::operator=(Packet other) {
    other.swap(*this);
    return *this;
}

void Packet::swap(Packet &other) {
    std::swap(m_fields, other.m_fields);
    std::swap(m_data, other.m_data);
    std::swap(m_parsePtr, other.m_parsePtr);
    std::swap(m_dataLength, other.m_dataLength);
}

void Packet::CopyToBuff(byte *ptr) {
    m_fields[0].SetVarInt(GetLength() - m_fields[0].GetLength());
    for (int i = 0; i < m_fields.size(); i++) {
        m_fields[i].CopyToBuff(ptr);
        ptr += m_fields[i].GetLength();
    }
}

void Packet::ParseField(FieldType type, size_t len) {
    if (type == ByteArray && len == 0)
        throw 118;
    Field f = FieldParser::Parse(type, m_parsePtr, len);
    m_fields.push_back(f);
    m_parsePtr += f.GetLength();
    if (m_parsePtr == m_data + m_dataLength) {
        delete[] m_data;
        m_data = nullptr;
        m_dataLength = 0;
        m_parsePtr = nullptr;
    }
}

Packet::Packet(byte *data) {
    Field fLen = FieldParser::Parse(VarInt, data);
    data += fLen.GetLength();
    Field fId = FieldParser::Parse(VarInt, data);
    data += fId.GetLength();
    m_dataLength = fLen.GetVarInt() - fId.GetLength();
    m_data = new byte[m_dataLength];
    std::copy(data,data+m_dataLength,m_data);
    m_parsePtr = m_data;
    m_fields.push_back(fLen);
    m_fields.push_back(fId);
}

Field &Packet::GetField(int id) {
    if (id < -2 || id >= m_fields.size() - 2)
        throw 111;
    return m_fields[id + 2];
}

size_t Packet::GetLength() {
    size_t len = 0;
    for (int i = 0; i < m_fields.size(); i++)
        len += m_fields[i].GetLength();
    return len + m_dataLength;
}

void Packet::AddField(Field field) {
    m_fields.push_back(field);
}

int Packet::GetId() {
    return m_fields[1].GetVarInt();
}

Packet::Packet(const Packet &other) {
    if (other.m_dataLength > 0) {
        m_dataLength = other.m_dataLength;
        m_data = new byte[m_dataLength];
        m_parsePtr = m_data + (other.m_data - other.m_parsePtr);
        std::copy(other.m_data, other.m_data + m_dataLength, m_data);
    }
    m_fields = other.m_fields;
}

void Packet::ParseFieldArray(Field &field, FieldType type, size_t len) {
    Field f = FieldParser::Parse(type, m_parsePtr, len);
    field.Attach(f);
    m_parsePtr += f.GetLength();
    if (m_parsePtr == m_data + m_dataLength) {
        delete[] m_data;
        m_data = nullptr;
        m_dataLength = 0;
        m_parsePtr = nullptr;
    }
}
