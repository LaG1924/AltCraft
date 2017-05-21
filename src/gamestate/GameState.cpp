#include "GameState.hpp"
#include <nlohmann/json.hpp>
#include "../packet/PacketParser.hpp"
#include "../packet/PacketBuilder.hpp"

GameState::GameState(NetworkClient *Net) : nc(Net) {
    Packet *response = nc->GetPacket();
    if (response->GetId() != 0x02) {
        std::cout << response->GetId() << std::endl;
        throw 127;
    }
    PacketParser::Parse(*response, Login);
    g_PlayerUuid = response->GetField(0).GetString();
    g_PlayerName = response->GetField(1).GetString();
    delete response;
    m_networkState = ConnectionState::Play;
    LOG(INFO) << g_PlayerName << "'s UUID is " << g_PlayerUuid;
}

void GameState::Update() {
    Packet &packet = *nc->GetPacket();
    if (&packet == nullptr)
        return;

    nlohmann::json json;

    PacketParser::Parse(packet, m_networkState);

    switch (packet.GetId()) {
        case 0x23:
            g_PlayerEid = packet.GetField(0).GetInt();
            g_Gamemode = (packet.GetField(1).GetUByte() & 0b11111011);
            g_Dimension = packet.GetField(2).GetInt();
            g_Difficulty = packet.GetField(3).GetUByte();
            g_MaxPlayers = packet.GetField(4).GetUByte();
            g_LevelType = packet.GetField(5).GetString();
            g_ReducedDebugInfo = packet.GetField(6).GetBool();
            LOG(INFO) << "Gamemode is " << g_Gamemode << ", Difficulty is " << (int) g_Difficulty
                      << ", Level Type is " << g_LevelType;
            break;
        case 0x0D:
            g_Difficulty = packet.GetField(0).GetUByte();
            std::cout << "Difficulty now is " << (int) g_Difficulty << std::endl;
            break;
        case 0x43:
            g_SpawnPosition = packet.GetField(0).GetPosition();
            LOG(INFO) << "Spawn position is " << g_SpawnPosition.GetX() << "," << g_SpawnPosition.GetY() << ","
                      << g_SpawnPosition.GetZ();
            break;
        case 0x2B:
            g_PlayerInvulnerable = (packet.GetField(0).GetByte() & 0x01) != 0;
            g_PlayerFlying = (packet.GetField(0).GetByte() & 0x02) != 0;
            g_PlayerAllowFlying = (packet.GetField(0).GetByte() & 0x04) != 0;
            g_PlayerCreativeMode = (packet.GetField(0).GetByte() & 0x08) != 0;
            g_PlayerFlyingSpeed = packet.GetField(1).GetFloat();
            g_PlayerFovModifier = packet.GetField(2).GetFloat();
            LOG(INFO) << "FOV modifier is " << g_PlayerFovModifier;
            break;
        case 0x2E:
            if ((packet.GetField(5).GetByte() & 0x10) != 0) {
                g_PlayerPitch += packet.GetField(4).GetFloat();
            } else {
                g_PlayerPitch = packet.GetField(4).GetFloat();
            };

            if ((packet.GetField(5).GetByte() & 0x08) != 0) {
                g_PlayerYaw += packet.GetField(3).GetFloat();
            } else {
                g_PlayerYaw = packet.GetField(3).GetFloat();
            }

            if ((packet.GetField(5).GetByte() & 0x01) != 0) {
                g_PlayerX += packet.GetField(0).GetDouble();
            } else {
                g_PlayerX = packet.GetField(0).GetDouble();
            }

            if ((packet.GetField(5).GetByte() & 0x02) != 0) {
                g_PlayerY += packet.GetField(1).GetDouble();
            } else {
                g_PlayerY = packet.GetField(1).GetDouble();
            }

            if ((packet.GetField(5).GetByte() & 0x04) != 0) {
                g_PlayerZ += packet.GetField(2).GetDouble();
            } else {
                g_PlayerZ = packet.GetField(2).GetDouble();
            }

            g_IsGameStarted = true;
            nc->AddPacketToQueue(PacketBuilder::CPlay0x03(0));
            nc->AddPacketToQueue(PacketBuilder::CPlay0x00(packet.GetField(6).GetVarInt()));
            LOG(INFO) << "Game is started!";
            LOG(INFO) << "PlayerPos is " << g_PlayerX << ", " << g_PlayerY << ", " << g_PlayerZ << "\tAngle: "
                      << g_PlayerYaw
                      << "," << g_PlayerPitch;
            break;
        case 0x1A:
            json = nlohmann::json::parse(packet.GetField(0).GetString());
            std::cout << "Disconnect reason: " << json["text"].get<std::string>() << std::endl;
            throw 119;
            break;
        case 0x20:
            world.ParseChunkData(packet);
            break;
        case 0x07:
            LOG(INFO) << "Statistics: ";
            for (int i = 0; i < packet.GetField(0).GetVarInt(); i++) {
                LOG(INFO) << "\t" << packet.GetField(1).GetArray()[0].GetString() << ": "
                          << packet.GetField(1).GetArray()[1].GetVarInt();
            }
            break;
        default:
            break;
    }
    if (g_IsGameStarted) {
        std::chrono::steady_clock clock;
        static auto timeOfPreviousSendedPpalPacket(clock.now());
        std::chrono::duration<double, std::milli> delta = clock.now() - timeOfPreviousSendedPpalPacket;
        if (delta.count() >= 50) {
            nc->AddPacketToQueue(
                    PacketBuilder::CPlay0x0D(g_PlayerX, g_PlayerY, g_PlayerZ, g_PlayerYaw, g_PlayerPitch, true));
            timeOfPreviousSendedPpalPacket = clock.now();
        }
    }

    delete &packet;
}
