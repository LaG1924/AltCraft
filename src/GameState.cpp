#include "GameState.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <easylogging++.h>

#include "Event.hpp"
#include "Packet.hpp"

void GameState::Update(float deltaTime) {
	if (g_IsGameStarted) {
		std::chrono::steady_clock clock;
		static auto timeOfPreviousSendedPacket(clock.now());
        auto delta = clock.now() - timeOfPreviousSendedPacket;
		using namespace std::chrono_literals;
        if (delta >= 50ms) {
            auto packetToSend = std::make_shared<PacketPlayerPositionAndLookSB>(player->pos.x, player->pos.y, player->pos.z, player->yaw, player->pitch, player->onGround);
            auto packet = std::static_pointer_cast<Packet>(packetToSend);
            PUSH_EVENT("SendPacket",packet);
            timeOfPreviousSendedPacket = clock.now();
        }

        bool prevOnGround = player->onGround;
        world.UpdatePhysics(deltaTime);
        if (player->onGround != prevOnGround) {
            auto updatePacket = std::make_shared<PacketPlayerPosition>(player->pos.x, player->pos.y, player->pos.z, player->onGround);
            auto packet = std::static_pointer_cast<Packet>(updatePacket);
            PUSH_EVENT("SendPacket",packet);
        }


        double playerYaw = Entity::DecodeYaw(player->yaw);
        double playerPitch = Entity::DecodePitch(player->pitch);

        glm::vec3 direction;
        direction.x = cos(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
        direction.y = sin(glm::radians(playerPitch));
        direction.z = sin(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));

        RaycastResult raycast = world.Raycast(player->pos + player->EyeOffset, direction);
        selectedBlock = raycast.isHit ? raycast.hitBlock : Vector(0,0,0);
        distanceToSelectedBlock = raycast.isHit ? (player->pos - raycast.hitPos).GetLength() : 0.0f;
        raycastHit = raycast.hitPos;
	}
}

void GameState::UpdatePacket(std::shared_ptr<Packet> ptr)
{
    switch ((PacketNamePlayCB)ptr->GetPacketId()) {
        case SpawnObject: {
            auto packet = std::static_pointer_cast<PacketSpawnObject>(ptr);
            Entity entity = CreateObject(static_cast<ObjectType>(packet->Type));
            entity.entityId = packet->EntityId;
            entity.pos = VectorF(packet->X, packet->Y, packet->Z);
            entity.uuid = packet->ObjectUuid;
            entity.vel = Entity::DecodeVelocity(packet->VelocityX, packet->VelocityY, packet->VelocityZ);
            entity.yaw = packet->Yaw / 256.0;
            entity.pitch = packet->Pitch / 256.0;
            entity.renderColor = glm::vec3(0, 1, 0);
            world.AddEntity(entity);
            PUSH_EVENT("EntityChanged", entity.entityId);
            break;
        }
        case SpawnExperienceOrb:
            break;
        case SpawnGlobalEntity:
            break;
        case SpawnMob: {
            auto packet = std::static_pointer_cast<PacketSpawnMob>(ptr);
            Entity entity;
            entity.entityId = packet->EntityId;
            entity.pos = VectorF(packet->X, packet->Y, packet->Z);
            entity.uuid = packet->EntityUuid;
            entity.vel = Entity::DecodeVelocity(packet->VelocityX, packet->VelocityY, packet->VelocityZ);
            entity.yaw = packet->Yaw / 256.0;
            entity.pitch = packet->Pitch / 256.0;
            entity.renderColor = glm::vec3(0, 0, 1);
            world.AddEntity(entity);
            PUSH_EVENT("EntityChanged", entity.entityId);
            break;
        }
        case SpawnPainting:
            break;
        case SpawnPlayer: {
            auto packet = std::static_pointer_cast<PacketSpawnPlayer>(ptr);
            Entity entity;
            entity.entityId = packet->EntityId;
            entity.pos = VectorF(packet->X, packet->Y, packet->Z);
            entity.uuid = packet->PlayerUuid;
            entity.yaw = packet->Yaw / 256.0;
            entity.pitch = packet->Pitch / 256.0;
            entity.renderColor = glm::vec3(1, 0, 0);
            entity.height = 1.8;
            entity.width = 0.6;
            world.AddEntity(entity);
            PUSH_EVENT("EntityChanged", entity.entityId);
            break;
        }
        case AnimationCB:
            break;
        case Statistics:
            break;
        case BlockBreakAnimation:
            break;
        case UpdateBlockEntity:
            break;
        case BlockAction:
            break;
        case BlockChange: {
            auto packet = std::static_pointer_cast<PacketBlockChange>(ptr);
            world.ParseChunkData(packet);
            break;
        }
        case BossBar:
            break;
        case ServerDifficulty:
            break;
        case TabCompleteCB:
            break;
        case ChatMessageCB: {
            auto packet = std::static_pointer_cast<PacketChatMessageCB>(ptr);
            LOG(INFO) << "Message (" << int(packet->Position) << "): " << packet->JsonData.text;
            PUSH_EVENT("ChatMessageReceived", std::make_tuple(packet->JsonData, packet->Position));
            break;
        }
        case MultiBlockChange: {
            auto packet = std::static_pointer_cast<PacketMultiBlockChange>(ptr);
            world.ParseChunkData(packet);
            break;
        }
        case ConfirmTransactionCB: {
            auto packet = std::static_pointer_cast<PacketConfirmTransactionCB>(ptr);
            if (packet->WindowId == 0) {
                try {
                    playerInventory.ConfirmTransaction(*packet);
                } catch (std::exception &e) {
                    PUSH_EVENT("Disconnected", std::string("Transaction failed"));
                }
            }
            break;
        }
        case CloseWindowCB:
            break;
        case OpenWindow: {
            auto packet = std::static_pointer_cast<PacketOpenWindow>(ptr);

            LOG(INFO) << "Open new window " << packet->WindowTitle << ": " << packet->WindowId;
            break;
        }
        case WindowItems: {
            auto packet = std::static_pointer_cast<PacketWindowItems>(ptr);
            if (packet->WindowId == 0) {
                playerInventory.WindowId = 0;
                playerInventory.slots = packet->SlotData;
            }
            break;
        }
        case WindowProperty:
            break;
        case SetSlot: {
            auto packet = std::static_pointer_cast<PacketSetSlot>(ptr);
            if (packet->WindowId == 0) {
                playerInventory.slots[packet->Slot] = packet->SlotData;
            }
            break;
        }
        case SetCooldown:
            break;
        case PluginMessageCB:
            break;
        case NamedSoundEffect:
            break;
        case DisconnectPlay: {
            auto packet = std::static_pointer_cast<PacketDisconnectPlay>(ptr);
            LOG(INFO) << "Disconnect reason: " << packet->Reason;
            PUSH_EVENT("Disconnected", packet->Reason);
            break;
        }
        case EntityStatus:
            break;
        case Explosion:
            break;
        case UnloadChunk: {
            auto packet = std::static_pointer_cast<PacketUnloadChunk>(ptr);
            world.ParseChunkData(packet);
            break;
        }
        case ChangeGameState:
            break;
        case KeepAliveCB:
            LOG(WARNING) << "Receive KeepAlive packet in GameState handler";
            break;
        case ChunkData: {
            auto packet = std::static_pointer_cast<PacketChunkData>(ptr);
            world.ParseChunkData(packet);
            break;
        }
        case Effect:
            break;
        case Particle:
            break;
        case JoinGame: {
            auto packet = std::static_pointer_cast<PacketJoinGame>(ptr);
            Entity entity;
            entity.entityId = packet->EntityId;
            entity.width = 0.6;
            entity.height = 1.8;
            world.AddEntity(entity);
            player = world.GetEntityPtr(entity.entityId);

            g_PlayerEid = packet->EntityId;
            g_Gamemode = (packet->Gamemode & 0b11111011);
            g_Dimension = packet->Dimension;
            g_Difficulty = packet->Difficulty;
            g_MaxPlayers = packet->MaxPlayers;
            g_LevelType = packet->LevelType;
            g_ReducedDebugInfo = packet->ReducedDebugInfo;
            LOG(INFO) << "Gamemode is " << g_Gamemode << ", Difficulty is " << (int) g_Difficulty
                      << ", Level Type is " << g_LevelType;
            PUSH_EVENT("PlayerConnected", 0);
            break;
        }
        case Map:
            break;
        case EntityRelativeMove: {
            auto packet = std::static_pointer_cast<PacketEntityRelativeMove>(ptr);
            Entity &entity = world.GetEntity(packet->EntityId);
            entity.pos = entity.pos + Entity::DecodeDeltaPos(packet->DeltaX, packet->DeltaY, packet->DeltaZ);
            if (entity.entityId != 0)
                LOG(INFO) << "M: " << packet->EntityId;
            break;
        }
        case EntityLookAndRelativeMove: {
            auto packet = std::static_pointer_cast<PacketEntityLookAndRelativeMove>(ptr);
            Entity &entity = world.GetEntity(packet->EntityId);
            entity.pos = entity.pos + Entity::DecodeDeltaPos(packet->DeltaX, packet->DeltaY, packet->DeltaZ);
            entity.pitch = packet->Pitch / 256.0;
            entity.yaw = packet->Yaw / 256.0;
            break;
        }
        case EntityLook: {
            auto packet = std::static_pointer_cast<PacketEntityLook>(ptr);
            Entity &entity = world.GetEntity(packet->EntityId);
            entity.pitch = packet->Pitch / 256.0;
            entity.yaw = packet->Yaw / 256.0;
            //LOG(INFO) << "L: " << packet->EntityId;
            break;
        }
        case EntityCB:
            break;
        case VehicleMove:
            break;
        case OpenSignEditor:
            break;
        case PlayerAbilitiesCB:
            break;
        case CombatEvent:
            break;
        case PlayerListItem:
            break;
        case PlayerPositionAndLookCB: {
            auto packet = std::static_pointer_cast<PacketPlayerPositionAndLookCB>(ptr);
            if ((packet->Flags & 0x10) != 0) {
                player->pitch += packet->Pitch;
            } else {
                player->pitch = packet->Pitch;
            };

            if ((packet->Flags & 0x08) != 0) {
                player->yaw += packet->Yaw;
            } else {
                player->yaw = packet->Yaw;
            }

            if ((packet->Flags & 0x01) != 0) {
                player->pos.x += packet->X;
            } else {
                player->pos.x = packet->X;
            }

            if ((packet->Flags & 0x02) != 0) {
                player->pos.y += packet->Y;
            } else {
                player->pos.y = packet->Y;
            }

            if ((packet->Flags & 0x04) != 0) {
                player->pos.z += packet->Z;
            } else {
                player->pos.z = packet->Z;
            }

            PUSH_EVENT("PlayerPosChanged", player->pos);
            LOG(INFO) << "PlayerPos is " << player->pos << "\t\tAngle: " << player->yaw << "," << player->pitch;;

            if (!g_IsGameStarted) {
                LOG(INFO) << "Game is started";
                PUSH_EVENT("RemoveLoadingScreen", 0);
            }

            g_IsGameStarted = true;

            auto packetResponse = std::make_shared<PacketTeleportConfirm>(packet->TeleportId);
            auto packetPerformRespawn = std::make_shared<PacketClientStatus>(0);

            PUSH_EVENT("SendPacket",std::static_pointer_cast<Packet>(packetResponse));
            PUSH_EVENT("SendPacket",std::static_pointer_cast<Packet>(packetPerformRespawn));
            break;
        }
        case UseBed:
            break;
        case UnlockRecipes:
            break;
        case DestroyEntities: {
            auto packet = std::static_pointer_cast<PacketDestroyEntities>(ptr);
            for (unsigned int entityId : packet->EntityIds) {
                world.DeleteEntity(entityId);
            }
            break;
        }
        case RemoveEntityEffect:
            break;
        case ResourcePackSend:
            break;
        case Respawn:
            break;
        case EntityHeadLook:
            break;
        case SelectAdvancementTab:
            break;
        case WorldBorder:
            break;
        case Camera:
            break;
        case HeldItemChangeCB:
            break;
        case DisplayScoreboard:
            break;
        case EntityMetadata:
            break;
        case AttachEntity:
            break;
        case EntityVelocity: {
            auto packet = std::static_pointer_cast<PacketEntityVelocity>(ptr);
            Entity &entity = world.GetEntity(packet->EntityId);
            entity.vel = Entity::DecodeVelocity(packet->VelocityX, packet->VelocityY, packet->VelocityZ);
            break;
        }
        case EntityEquipment:
            break;
        case SetExperience:
            break;
        case UpdateHealth: {
            auto packet = std::static_pointer_cast<PacketUpdateHealth>(ptr);
            g_PlayerHealth = packet->Health;
            if (g_PlayerHealth <= 0) {
                LOG(INFO) << "Player is dead. Respawning...";
                auto packetPerformRespawn = std::make_shared<PacketClientStatus>(0);
                PUSH_EVENT("SendPacket",std::static_pointer_cast<Packet>(packetPerformRespawn));
            }
            break;
        }
        case ScoreboardObjective:
            break;
        case SetPassengers:
            break;
        case Teams:
            break;
        case UpdateScore:
            break;
        case SpawnPosition: {
            auto packet = std::static_pointer_cast<PacketSpawnPosition>(ptr);
            g_SpawnPosition = packet->Location;
            LOG(INFO) << "Spawn position is " << g_SpawnPosition.x << " " << g_SpawnPosition.y << " "
                      << g_SpawnPosition.z;
            break;
        }
        case TimeUpdate: {
            auto packet = std::static_pointer_cast<PacketTimeUpdate>(ptr);
            WorldAge = packet->WorldAge;
            TimeOfDay = packet->TimeOfDay;
            break;
        }
        case Title:
            break;
        case SoundEffect:
            break;
        case PlayerListHeaderAndFooter:
            break;
        case CollectItem:
            break;
        case EntityTeleport: {
            auto packet = std::static_pointer_cast<PacketEntityTeleport>(ptr);
            Entity &entity = world.GetEntity(packet->EntityId);
            entity.pos = VectorF(packet->X, packet->Y, packet->Z);
            entity.pitch = packet->Pitch / 256.0;
            entity.yaw = packet->Yaw / 256.0;
            break;
        }
        case Advancements:
            break;
        case EntityProperties:
            break;
        case EntityEffect:
            break;
    }
    while (!playerInventory.pendingTransactions.empty()) {
        auto packet = std::make_shared<PacketClickWindow>(playerInventory.pendingTransactions.front());
        playerInventory.pendingTransactions.pop();
        PUSH_EVENT("SendPacket",std::static_pointer_cast<Packet>(packet));
    }
}

void GameState::HandleMovement(GameState::Direction direction, float deltaTime) {
    if (!g_IsGameStarted)
        return;
    const double playerSpeed = 43;

    float velocity = playerSpeed * deltaTime;

    double playerYaw = Entity::DecodeYaw(player->yaw);
    double playerPitch = Entity::DecodePitch(player->pitch);

    glm::vec3 front, right, worldUp, up;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    front.x = cos(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
    front.y = 0;
    front.z = sin(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));

    glm::vec3 vel = player->vel.glm();
    switch (direction) {
    case FORWARD:
        vel += front * velocity;
        break;
    case BACKWARD:
        vel -= front * velocity;
        break;
    case RIGHT:
        vel += right * velocity;
        break;
    case LEFT:
        vel -= right * velocity;
        break;
    case JUMP:
        if (player->onGround) {
            vel.y += 10;
            player->onGround = false;
        }
        break;
    }
    player->vel = VectorF(vel.x, vel.y, vel.z);
}

void GameState::HandleRotation(double yaw, double pitch) {
    if (!g_IsGameStarted)
        return;
    double playerYaw = Entity::DecodeYaw(player->yaw);
    double playerPitch = Entity::DecodePitch(player->pitch);
    playerYaw += yaw;
    playerPitch += pitch;
    if (playerPitch > 89.0)
        playerPitch = 89.0;
    if (playerPitch < -89.0)
        playerPitch = -89.0;
    player->yaw = Entity::EncodeYaw(playerYaw);
    player->pitch = Entity::EncodePitch(playerPitch);
}

glm::mat4 GameState::GetViewMatrix() {
    double playerYaw = Entity::DecodeYaw(player->yaw);
    double playerPitch = Entity::DecodePitch(player->pitch);
    glm::vec3 front, right, worldUp, up;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    front.x = cos(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
    front.y = sin(glm::radians(playerPitch));
    front.z = sin(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
    front = glm::normalize(front);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));

    glm::vec3 eyePos = player->pos.glm();
    eyePos += player->EyeOffset.glm();
    return glm::lookAt(eyePos, eyePos + front, up);
}

void GameState::StartDigging() {
    auto packetStart = std::make_shared<PacketPlayerDigging>(0,selectedBlock,1);
    auto packetStop = std::make_shared<PacketPlayerDigging>(2,selectedBlock,1);
    auto packet = std::static_pointer_cast<Packet>(packetStart);
    PUSH_EVENT("SendPacket",packet);
    packet = std::static_pointer_cast<Packet>(packetStop);
    PUSH_EVENT("SendPacket",packet);
}

void GameState::StopDigging() {

}
