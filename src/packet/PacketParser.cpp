#include "PacketParser.hpp"

void PacketParser::Parse(Packet &packet, ConnectionState state, bool ClientBound) {
    if (ClientBound) {
        switch (state) {
            case Login:
                ParseLogin(packet);
                break;
            case Handshaking:
                break;
            case Play:
                ParsePlay(packet);
                break;
            case Status:

                break;
        }
    } else {
        ParseServerBound(packet, state);
    }
}

void PacketParser::ParseServerBound(Packet &packet, ConnectionState state) {
    throw 107;
}

void PacketParser::ParseLogin(Packet &packet) {
    switch (packet.GetId()) {
        case 0x00:
            ParseLogin0x00(packet);
            break;
        case 0x02:
            ParseLogin0x02(packet);
            break;
        default:
        {
            int i = packet.GetId();
            //throw 112;
        }
    }
}

void PacketParser::ParsePlay(Packet &packet) {
    switch (packet.GetId()) {
        case 0x23:
            ParsePlay0x23(packet);
            break;
        case 0x1F:
            ParsePlay0x1F(packet);
            break;
        case 0x0D:
            ParsePlay0x0D(packet);
            break;
        case 0x2B:
            ParsePlay0x2B(packet);
            break;
        case 0x43:
            ParsePlay0x43(packet);
            break;
        case 0x2E:
            ParsePlay0x2E(packet);
            break;
        case 0x1A:
            ParsePlay0x1A(packet);
            break;
        case 0x20:
            ParsePlay0x20(packet);
            break;
        case 0x07:
            ParsePlay0x07(packet);
        default:
            //throw 113;
            break;
    }
}

void PacketParser::ParseLogin0x00(Packet &packet) {
    packet.ParseField(String);
}

void PacketParser::ParseLogin0x02(Packet &packet) {
    packet.ParseField(String);
    packet.ParseField(String);
}

void PacketParser::ParsePlay0x23(Packet &packet) {
    packet.ParseField(Int);
    packet.ParseField(UnsignedByte);
    packet.ParseField(Int);
    packet.ParseField(UnsignedByte);
    packet.ParseField(UnsignedByte);
    packet.ParseField(String);
    packet.ParseField(Boolean);
}

void PacketParser::ParsePlay0x1F(Packet &packet) {
    packet.ParseField(VarInt);
}

void PacketParser::ParsePlay0x0D(Packet &packet) {
    packet.ParseField(UnsignedByte);
}

void PacketParser::ParsePlay0x2B(Packet &packet) {
    packet.ParseField(Byte8_t);
    packet.ParseField(Float);
    packet.ParseField(Float);
}

void PacketParser::ParsePlay0x43(Packet &packet) {
    packet.ParseField(Position);
}

void PacketParser::ParsePlay0x2E(Packet &packet) {
    packet.ParseField(Double);
    packet.ParseField(Double);
    packet.ParseField(Double);
    packet.ParseField(Float);
    packet.ParseField(Float);
    packet.ParseField(Byte8_t);
    packet.ParseField(VarInt);
}

void PacketParser::ParsePlay0x1A(Packet &packet) {
    packet.ParseField(String);
}

void PacketParser::ParsePlay0x20(Packet &packet) {
    packet.ParseField(Int);
    packet.ParseField(Int);
    packet.ParseField(Boolean);
    packet.ParseField(VarInt);
    packet.ParseField(VarInt);
    packet.ParseField(ByteArray, packet.GetField(4).GetVarInt());
    packet.ParseField(VarInt);
    //packet.ParseField(NbtTag);
    //packet.GetField(7).SetArray(packet.GetField(6).GetVarInt());
}

void PacketParser::ParsePlay0x07(Packet &packet) {
    packet.ParseField(VarInt);
    packet.AddField(Field());
    for (int i=0;i<packet.GetField(0).GetVarInt();i++){
        packet.ParseFieldArray(packet.GetField(1), String, 0);
        packet.ParseFieldArray(packet.GetField(1), VarInt, 0);
    }
}
