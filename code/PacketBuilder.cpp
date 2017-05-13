#include "PacketBuilder.hpp"

Packet PacketBuilder::CHandshaking0x00(int protocolVerison, std::string address, unsigned short port, int nextState) {
    Packet handshakePacket(0);
    Field fProtocol;
    fProtocol.SetVarInt(protocolVerison);
    Field fAddress;
    fAddress.SetString(address);
    Field fPort;
    fPort.SetUShort(port);
    Field fNextState;
    fNextState.SetVarInt(nextState);
    handshakePacket.AddField(fProtocol);
    handshakePacket.AddField(fAddress);
    handshakePacket.AddField(fPort);
    handshakePacket.AddField(fNextState);
    return handshakePacket;
}

Packet PacketBuilder::CPlay0x0B(int keepAliveId) {
    Packet keepAlivePacket(0x0B);
    Field fKeepAlive;
    fKeepAlive.SetVarInt(keepAliveId);
    keepAlivePacket.AddField(fKeepAlive);
    return keepAlivePacket;
}

Packet PacketBuilder::CPlay0x03(int actionId) {
    Packet clientStatusPacket(0x03);
    Field fActionId;
    fActionId.SetVarInt(actionId);
    clientStatusPacket.AddField(fActionId);
    return clientStatusPacket;
}

Packet PacketBuilder::CPlay0x00(int teleportId) {
    Packet teleportConfirmPacket(0x00);
    Field fTeleportId;
    fTeleportId.SetVarInt(teleportId);
    teleportConfirmPacket.AddField(fTeleportId);
    return teleportConfirmPacket;
}

Packet PacketBuilder::CPlay0x0D(double x, double y, double z, float yaw, float pitch, bool onGround) {
    Packet playerPositionAndLookPacket(0x0D);
    Field fX;
    Field fY;
    Field fZ;
    Field fYaw;
    Field fPitch;
    Field fOnGround;
    fX.SetDouble(x);
    fY.SetDouble(y);
    fZ.SetDouble(z);
    fYaw.SetFloat(yaw);
    fPitch.SetFloat(pitch);
    fOnGround.SetBool(onGround);
    playerPositionAndLookPacket.AddField(fX);
    playerPositionAndLookPacket.AddField(fY);
    playerPositionAndLookPacket.AddField(fZ);
    playerPositionAndLookPacket.AddField(fYaw);
    playerPositionAndLookPacket.AddField(fPitch);
    playerPositionAndLookPacket.AddField(fOnGround);
    return playerPositionAndLookPacket;
}
