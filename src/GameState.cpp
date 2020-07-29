#include "GameState.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <easylogging++.h>
#include <optick.h>

#include "Event.hpp"
#include "Packet.hpp"

void GameState::Update(double deltaTime) {
	OPTICK_EVENT();
	if (!gameStatus.isGameStarted)
		return;

	std::chrono::steady_clock clock;
	static auto timeOfPreviousSendedPacket(clock.now());
	auto delta = clock.now() - timeOfPreviousSendedPacket;
	using namespace std::chrono_literals;
	if (delta >= 50ms) {
		auto packetToSend = std::make_shared<PacketPlayerPositionAndLookSB>(
			player->pos.x, player->pos.y, player->pos.z,
			player->yaw, player->pitch, player->onGround);

		auto packet = std::static_pointer_cast<Packet>(packetToSend);
		PUSH_EVENT("SendPacket", packet);
		timeOfPreviousSendedPacket = clock.now();
	}

	bool prevOnGround = player->onGround;
	world.UpdatePhysics(deltaTime);
	if (player->onGround != prevOnGround) {
		auto updatePacket = std::make_shared<PacketPlayerPosition>(
			player->pos.x, player->pos.y,
			player->pos.z, player->onGround);

		auto packet = std::static_pointer_cast<Packet>(updatePacket);
		PUSH_EVENT("SendPacket", packet);
	}


	double playerYaw = Entity::DecodeYaw(player->yaw);
	double playerPitch = Entity::DecodePitch(player->pitch);

	glm::vec3 direction;
	direction.x = cos(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
	direction.y = sin(glm::radians(playerPitch));
	direction.z = sin(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));

	RaycastResult raycast = world.Raycast(player->pos + player->EyeOffset, direction);
	if (raycast.isHit != selectionStatus.isBlockSelected || ((raycast.isHit == true && selectionStatus.isBlockSelected == true) &&
		selectionStatus.selectedBlock != raycast.hitBlock)) {
		PUSH_EVENT("SelectedBlockChanged", 0);
	}

	if (raycast.isHit) {
		selectionStatus.selectedBlock = raycast.hitBlock;
		selectionStatus.distanceToSelectedBlock = (player->pos - raycast.hitPos).GetLength();
	}
	else {
		selectionStatus.selectedBlock = Vector(0, 0, 0);
		selectionStatus.distanceToSelectedBlock = 0.0f;
	}

	selectionStatus.isBlockSelected = raycast.isHit;
	selectionStatus.raycastHit = raycast.hitPos;

	if (timeStatus.doDaylightCycle)
		timeStatus.interpolatedTimeOfDay += 20.0 * deltaTime;
}

void GameState::UpdatePacket(std::shared_ptr<Packet> ptr) {
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
			LOG(INFO) << "Message (" << int(packet->Position) << "): " << packet->JsonData.ToPlainText();
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
				}
				catch (std::exception &e) {
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

		case KeepAliveCB: {
			LOG(WARNING) << "Receive KeepAlive packet in GameState handler";
			break;
		}

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
			world = World(packet->Dimension);
			world.AddEntity(entity);
			player = world.GetEntityPtr(entity.entityId);

			playerStatus.eid = packet->EntityId;
			gameStatus.gamemode = (packet->Gamemode & 0b11111011);
			gameStatus.dimension = packet->Dimension;
			gameStatus.difficulty = packet->Difficulty;
			gameStatus.maxPlayers = packet->MaxPlayers;
			gameStatus.levelType = packet->LevelType;
			gameStatus.reducedDebugInfo = packet->ReducedDebugInfo;
			LOG(INFO) << "Gamemode is " << gameStatus.gamemode << ", Difficulty is " << (int)gameStatus.difficulty
				<< ", Level Type is " << gameStatus.levelType;
			PUSH_EVENT("PlayerConnected", 0);

			auto packetSettings = std::make_shared<PacketClientSettings>("en_us", 0x14, 0, true, 0x7F, 1);
			PUSH_EVENT("SendPacket", std::static_pointer_cast<Packet>(packetSettings));

			std::string brandStr("\x08""AltCraft");
			std::vector<unsigned char> brandData;
			std::copy(brandStr.begin(), brandStr.end(), std::back_inserter(brandData));
			auto packetPluginBrand = std::make_shared<PacketPluginMessageSB>("MC|Brand", brandData);
			PUSH_EVENT("SendPacket", std::static_pointer_cast<Packet>(packetPluginBrand));

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
			}
			else {
				player->pitch = packet->Pitch;
			}

			if ((packet->Flags & 0x08) != 0) {
				player->yaw += packet->Yaw;
			}
			else {
				player->yaw = packet->Yaw;
			}

			if ((packet->Flags & 0x01) != 0) {
				player->pos.x += packet->X;
			}
			else {
				player->pos.x = packet->X;
			}

			if ((packet->Flags & 0x02) != 0) {
				player->pos.y += packet->Y;
			}
			else {
				player->pos.y = packet->Y;
			}

			if ((packet->Flags & 0x04) != 0) {
				player->pos.z += packet->Z;
			}
			else {
				player->pos.z = packet->Z;
			}

			PUSH_EVENT("PlayerPosChanged", player->pos);
			LOG(INFO) << "PlayerPos is " << player->pos << "\t\tAngle: " << player->yaw << "," << player->pitch;;

			if (!gameStatus.isGameStarted) {
				LOG(INFO) << "Game is started";
				PUSH_EVENT("RemoveLoadingScreen", 0);
			}

			gameStatus.isGameStarted = true;

			auto packetResponse = std::make_shared<PacketTeleportConfirm>(packet->TeleportId);
			auto packetPerformRespawn = std::make_shared<PacketClientStatus>(0);

			PUSH_EVENT("SendPacket", std::static_pointer_cast<Packet>(packetResponse));
			PUSH_EVENT("SendPacket", std::static_pointer_cast<Packet>(packetPerformRespawn));
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
		case Respawn: {
			auto packet = std::static_pointer_cast<PacketRespawn>(ptr);
			Entity entity;
			entity.entityId = player->entityId;
			entity.width = 0.6;
			entity.height = 1.8;
			world = World(packet->Dimension);
			world.AddEntity(entity);
			player = world.GetEntityPtr(entity.entityId);

			gameStatus.gamemode = (packet->Gamemode & 0b11111011);
			gameStatus.dimension = packet->Dimension;
			gameStatus.difficulty = packet->Difficulty;
			gameStatus.levelType = packet->LevelType;
			break;
		}
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
			playerStatus.health = packet->Health;
			if (playerStatus.health <= 0) {
				LOG(INFO) << "Player is dead. Respawning...";
				auto packetPerformRespawn = std::make_shared<PacketClientStatus>(0);
				PUSH_EVENT("SendPacket", std::static_pointer_cast<Packet>(packetPerformRespawn));
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
			gameStatus.spawnPosition = packet->Location;
			LOG(INFO) << "Spawn position is " << gameStatus.spawnPosition;
			break;
		}

		case TimeUpdate: {
			auto packet = std::static_pointer_cast<PacketTimeUpdate>(ptr);
			timeStatus.doDaylightCycle = timeStatus.timeOfDay != packet->TimeOfDay;
			timeStatus.worldAge = packet->WorldAge;
			timeStatus.timeOfDay = packet->TimeOfDay;
			timeStatus.interpolatedTimeOfDay = timeStatus.timeOfDay;
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
		PUSH_EVENT("SendPacket", std::static_pointer_cast<Packet>(packet));
	}
}

void GameState::HandleMovement(GameState::MoveType direction, float deltaTime) {
	if (!gameStatus.isGameStarted)
		return;

	const double playerSpeed = 43;

	float velocity = playerSpeed * deltaTime;

	double playerYaw = Entity::DecodeYaw(player->yaw);
	double playerPitch = Entity::DecodePitch(player->pitch);

	glm::vec3 front, right, worldUp, up;
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	front.x = cos(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
	front.y = player->isFlying ? sin(glm::radians(playerPitch)) : 0;
	front.z = sin(glm::radians(playerYaw)) * cos(glm::radians(playerPitch));
	front = glm::normalize(front);
	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));

	glm::vec3 vel = player->vel.glm();
	switch (direction) {
		case FORWARD: {
			vel += front * velocity;
			break;
		}

		case BACKWARD: {
			vel -= front * velocity;
			break;
		}

		case RIGHT: {
			vel += right * velocity;
			break;
		}

		case LEFT: {
			vel -= right * velocity;
			break;
		}

		case JUMP:
			if (player->onGround && !player->isFlying) {
				vel.y += 10;
				player->onGround = false;
			}
			else
				if (player->isFlying) {
					vel += up * velocity;
				}
			break;
	}
	player->vel = VectorF(vel.x, vel.y, vel.z);
}

void GameState::HandleRotation(double yaw, double pitch) {
	if (!gameStatus.isGameStarted)
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

// TODO: it should actually be something like this:
//    function start_digging():
//        send_packet(packet_type=start_digging_packet)
//        delay(time=selected_block_dig_time, action=finish_digging)
void GameState::StartDigging() {
	if (!selectionStatus.isBlockSelected)
		return;

	auto packetStart = std::make_shared<PacketPlayerDigging>(0, selectionStatus.selectedBlock, 1);
	auto packet = std::static_pointer_cast<Packet>(packetStart);
	PUSH_EVENT("SendPacket", packet);

	FinishDigging();
}

void GameState::FinishDigging() {
	auto packetFinish = std::make_shared<PacketPlayerDigging>(2, selectionStatus.selectedBlock, 1);
	auto packet = std::static_pointer_cast<Packet>(packetFinish);
	PUSH_EVENT("SendPacket", packet);
}

// TODO: it should actually be something like this:
//    function cancel_digging():
//        if finish_digging is in delayed_actions:
//            send_packet(packet_type=start_digging_packet)
//            remove_delayed_action(finish_digging)
void GameState::CancelDigging() {
	auto packetCancel = std::make_shared<PacketPlayerDigging>(1, selectionStatus.selectedBlock, 1);
	auto packet = std::static_pointer_cast<Packet>(packetCancel);
	PUSH_EVENT("SendPacket", packet);
}

BlockFacing detectHitFace(VectorF raycastHit, Vector selectedBlock) {
	auto vec = VectorF(selectedBlock.x + .5, selectedBlock.y + .5, selectedBlock.z + .5) - raycastHit;

	// TODO: move these vectors to Vector.hpp
	static const auto vecUp = VectorF(0, 1, 0);
	static const auto vecRight = VectorF(1, 0, 0);
	static const auto vecForward = VectorF(0, 0, -1);

	const double up = vec.cosBetween(vecUp);
	const double down = -up;
	const double right = vec.cosBetween(vecRight);
	const double left = -right;
	const double forward = vec.cosBetween(vecForward);
	const double backward = -forward;

	const double min_cos = _min(up, down, right, left, forward, backward);
	if (min_cos == down)
		return BlockFacing::Bottom;
	else if (min_cos == up)
		return BlockFacing::Top;
	else if (min_cos == forward)
		return BlockFacing::North;
	else if (min_cos == backward)
		return BlockFacing::South;
	else if (min_cos == left)
		return BlockFacing::West;
	else return BlockFacing::East;
}

void GameState::PlaceBlock() {
	if (!selectionStatus.isBlockSelected)
		return;

	BlockFacing face = detectHitFace(selectionStatus.raycastHit, selectionStatus.selectedBlock);
	auto packetPlace = std::make_shared<PacketPlayerBlockPlacement>(
		selectionStatus.selectedBlock, (unsigned char)face, 0, 0, 0, 0);

	auto packet = std::static_pointer_cast<Packet>(packetPlace);
	PUSH_EVENT("SendPacket", packet);
}