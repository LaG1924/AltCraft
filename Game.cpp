#include "Game.hpp"
#include "PacketParser.hpp"
#include "PacketBuilder.hpp"
#include "json.hpp"

Game::Game() {
    m_display = new Display(1280, 720, "AltCraft", &m_world);
    m_nc = new NetworkClient("127.0.0.1", 25565, "HelloOne");
    Packet &response = *m_nc->GetPacket();
    if (response.GetId() != 0x02) {
        std::cout << response.GetId() << std::endl;
        throw 127;
    }
    PacketParser::Parse(response, Login);
    g_PlayerUuid = response.GetField(0).GetString();
    g_PlayerName = response.GetField(1).GetString();
    delete &response;
    m_networkState = ConnectionState::Play;
    std::cout << g_PlayerName << "'s UUID is " << g_PlayerUuid << std::endl;
}

Game::~Game() {
    std::cout << "Stopping game thread..." << std::endl;
    m_exit=true;
    m_gameThread.join();
    std::cout << "Stopping graphics..." << std::endl;
    delete m_display;
    std::cout << "Stopping network..." << std::endl;
    delete m_nc;
}

void Game::MainLoop() {
    while (!m_exit) {
        ParsePackets();
        if (m_display->IsClosed())
            m_exit = true;
    }
}

void Game::ParsePackets() {
    Packet *packetPtr = m_nc->GetPacket();
    if (!packetPtr) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(16ms);
        return;
    }
    Packet packet = *packetPtr;
    delete packetPtr;
    PacketParser::Parse(packet);
    nlohmann::json json;

    switch (packet.GetId()) {
        case 0x23:
            g_PlayerEid = packet.GetField(0).GetInt();
            g_Gamemode = (packet.GetField(1).GetUByte() & 0b11111011);
            g_Dimension = packet.GetField(2).GetInt();
            g_Difficulty = packet.GetField(3).GetUByte();
            g_MaxPlayers = packet.GetField(4).GetUByte();
            g_LevelType = packet.GetField(5).GetString();
            g_ReducedDebugInfo = packet.GetField(6).GetBool();
            std::cout << "Gamemode is " << (int) g_Gamemode << ", Difficulty is " << (int) g_Difficulty
                      << ", Level Type is " << g_LevelType << std::endl;
            break;
        case 0x0D:
            g_Difficulty = packet.GetField(0).GetUByte();
            std::cout << "Difficulty now is " << (int) g_Difficulty << std::endl;
            break;
        case 0x43:
            g_SpawnPosition = packet.GetField(0).GetPosition();
            std::cout << "Spawn position is " << g_SpawnPosition.GetX() << "," << g_SpawnPosition.GetY() << ","
                      << g_SpawnPosition.GetZ() << std::endl;
            break;
        case 0x2B:
            g_PlayerInvulnerable = (packet.GetField(0).GetByte() & 0x01) != 0;
            g_PlayerFlying = (packet.GetField(0).GetByte() & 0x02) != 0;
            g_PlayerAllowFlying = (packet.GetField(0).GetByte() & 0x04) != 0;
            g_PlayerCreativeMode = (packet.GetField(0).GetByte() & 0x08) != 0;
            g_PlayerFlyingSpeed = packet.GetField(1).GetFloat();
            g_PlayerFovModifier = packet.GetField(2).GetFloat();
            std::cout << "Fov modifier is " << g_PlayerFovModifier << std::endl;
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
            m_nc->AddPacketToQueue(PacketBuilder::CPlay0x03(0));
            m_nc->AddPacketToQueue(PacketBuilder::CPlay0x00(packet.GetField(6).GetVarInt()));
            std::cout << "Game is started! " << std::endl;
            std::cout << "PlayerPos is " << g_PlayerX << "," << g_PlayerY << "," << g_PlayerZ << "\t" << g_PlayerYaw
                      << "," << g_PlayerPitch << std::endl;
            m_display->SetPlayerPos(g_PlayerX, g_PlayerY);
            gameStartWaiter.notify_all();
            break;
        case 0x1A:
            json = nlohmann::json::parse(packet.GetField(0).GetString());
            std::cout << "Disconnect reason: " << json["text"].get<std::string>() << std::endl;
            throw 119;
            break;
        case 0x20:
            m_world.ParseChunkData(packet);
            break;
        case 0x07:
            std::cout << "Statistics:" << std::endl;
            //int items = packet.GetField(0).GetVarInt();
            for (int i = 0; i < packet.GetField(0).GetVarInt(); i++) {
                std::cout << "\t" << packet.GetField(1).GetArray()[0].GetString() << ": "
                          << packet.GetField(1).GetArray()[1].GetVarInt() << std::endl;
            }
            break;
        default:
            //std::cout << std::hex << packet.GetId() << std::dec << std::endl;
            break;
    }
    if (g_IsGameStarted) {
        std::chrono::steady_clock clock;
        static auto timeOfPreviousSendedPpalPacket(clock.now());
        std::chrono::duration<double, std::milli> delta = clock.now() - timeOfPreviousSendedPpalPacket;
        if (delta.count() >= 50) {
            m_nc->AddPacketToQueue(
                    PacketBuilder::CPlay0x0D(g_PlayerX, g_PlayerY, g_PlayerZ, g_PlayerYaw, g_PlayerPitch, true));
            timeOfPreviousSendedPpalPacket = clock.now();
            /*std::cout << "PlayerPos is " << g_PlayerX << "," << g_PlayerY << "," << g_PlayerZ << " " << g_PlayerYaw
                      << "," << g_PlayerPitch << std::endl;*/
        }
    }
}

void Game::Exec() {
    m_gameThread = std::thread(&Game::MainLoop, this);
    m_display->MainLoop();
}
