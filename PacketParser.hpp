#pragma once


#include "Packet.hpp"

class PacketParser {
public:
    static void Parse(Packet &packet, ConnectionState state = Play, bool ClientBound = true);

    static void ParseServerBound(Packet &packet, ConnectionState state);

    static void ParseLogin(Packet &packet);

    static void ParsePlay(Packet &packet);

    static void ParseLogin0x00(Packet &packet);

    static void ParseLogin0x02(Packet &packet);

    static void ParsePlay0x23(Packet &packet);

    static void ParsePlay0x1F(Packet &packet);

    static void ParsePlay0x0D(Packet &packet);

    static void ParsePlay0x2B(Packet &packet);

    static void ParsePlay0x43(Packet &packet);

    static void ParsePlay0x2E(Packet &packet);

    static void ParsePlay0x1A(Packet &packet);

    static void ParsePlay0x20(Packet &packet);

    static void ParsePlay0x07(Packet &packet);
};

