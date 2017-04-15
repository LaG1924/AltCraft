#pragma once


#include "Packet.hpp"

class PacketBuilder {
public:
    static Packet CHandshaking0x00(int protocolVerison, std::string address, unsigned short port, int nextState);
    static Packet CPlay0x0B(int keepAliveId);

    static Packet CPlay0x03(int actionId);

    static Packet CPlay0x00(int teleportId);

    static Packet CPlay0x0D(double x, double y, double z, float yaw, float pitch, bool onGround);
};

