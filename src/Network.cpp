#include "Network.hpp"

Network::Network(std::string address, unsigned short port) {
	try {
		socket = new Socket(address, port);
	} catch (std::exception &e) {
		LOG(WARNING) << "Connection failed: " << e.what();
		throw;
	}

	try {
		stream = new StreamSocket(socket);
	} catch (std::exception &e) {
		LOG(WARNING) << "Stream creation failed: " << e.what();
	}
}

Network::~Network() {
	delete stream;
	delete socket;
}

std::shared_ptr<Packet> Network::ReceivePacket(ConnectionState state) {
	int packetSize = stream->ReadVarInt();
	auto packetData = stream->ReadByteArray(packetSize);
	StreamBuffer streamBuffer(packetData.data(), packetData.size());
	int packetId = streamBuffer.ReadVarInt();
	auto packet = ReceivePacketByPacketId(packetId, state, streamBuffer);
	return packet;
}

void Network::SendPacket(Packet &packet) {
	StreamCounter packetSize;
	packetSize.WriteVarInt(packet.GetPacketId());
	packet.ToStream(&packetSize);
	stream->WriteVarInt(packetSize.GetCountedSize());
	stream->WriteVarInt(packet.GetPacketId());
	packet.ToStream(stream);
}

std::shared_ptr<Packet> Network::ReceivePacketByPacketId(int packetId, ConnectionState state, StreamInput &stream) {
	std::shared_ptr < Packet > packet(nullptr);
	switch (state) {
		case Handshaking:
			switch (packetId) {
				case PacketNameHandshakingCB::Handshake:
					packet = std::make_shared<PacketHandshake>();
					break;
			}
		case Login:
			switch (packetId) {
				case PacketNameLoginCB::LoginSuccess:
					packet = std::make_shared<PacketLoginSuccess>();
					break;
			}
			break;
		case Play:
			packet = ParsePacketPlay((PacketNamePlayCB) packetId);
			break;
		case Status:
			break;
	}
	if (packet.get() != nullptr)
		packet->FromStream(&stream);
	return packet;
}

std::shared_ptr<Packet> Network::ParsePacketPlay(PacketNamePlayCB id) {
	switch (id) {
		case SpawnObject:
            return std::make_shared<PacketSpawnObject>();
		case SpawnExperienceOrb:
			break;
		case SpawnGlobalEntity:
			break;
		case SpawnMob:
            return std::make_shared<PacketSpawnMob>();
		case SpawnPainting:
			break;
		case SpawnPlayer:
            return std::make_shared<PacketSpawnPlayer>();
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
		case BlockChange:
            return std::make_shared<PacketBlockChange>();
		case BossBar:
			break;
		case ServerDifficulty:
			break;
		case TabCompleteCB:
			break;
		case ChatMessageCB:
			break;
		case MultiBlockChange:
            return std::make_shared<PacketMultiBlockChange>();
		case ConfirmTransactionCB:
			break;
		case CloseWindowCB:
			break;
		case OpenWindow:
			break;
		case WindowItems:
			break;
		case WindowProperty:
			break;
		case SetSlot:
			break;
		case SetCooldown:
			break;
		case PluginMessageCB:
			break;
		case NamedSoundEffect:
			break;
		case DisconnectPlay:
			return std::make_shared<PacketDisconnectPlay>();
		case EntityStatus:
			break;
		case Explosion:
			break;
		case UnloadChunk:
            return std::make_shared<PacketUnloadChunk>();
		case ChangeGameState:
			break;
		case KeepAliveCB:
			return std::make_shared<PacketKeepAliveCB>();
		case ChunkData:
			return std::make_shared<PacketChunkData>();
		case Effect:
			break;
		case Particle:
			break;
		case JoinGame:
			return std::make_shared<PacketJoinGame>();
		case Map:
			break;
		case EntityRelativeMove:
            return std::make_shared<PacketEntityRelativeMove>();
		case EntityLookAndRelativeMove:
            return std::make_shared<PacketEntityLookAndRelativeMove>();
		case EntityLook:
            return std::make_shared<PacketEntityLook>();
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
		case PlayerPositionAndLookCB:
			return std::make_shared<PacketPlayerPositionAndLookCB>();
		case UseBed:
			break;
		case DestroyEntities:
            return std::make_shared<PacketDestroyEntities>();
		case RemoveEntityEffect:
			break;
		case ResourcePackSend:
			break;
		case Respawn:
			break;
		case EntityHeadLook:
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
		case EntityVelocity:
            return std::make_shared<PacketEntityVelocity>();
			break;
		case EntityEquipment:
			break;
		case SetExperience:
			break;
		case UpdateHealth:
			return std::make_shared<PacketUpdateHealth>();
		case ScoreboardObjective:
			break;
		case SetPassengers:
			break;
		case Teams:
			break;
		case UpdateScore:
			break;
		case SpawnPosition:
			return std::make_shared<PacketSpawnPosition>();
		case TimeUpdate:
            return std::make_shared<PacketTimeUpdate>();
		case Title:
			break;
		case SoundEffect:
			break;
		case PlayerListHeaderAndFooter:
			break;
		case CollectItem:
			break;
		case EntityTeleport:
            return std::make_shared<PacketEntityTeleport>();
			break;
		case EntityProperties:
			break;
		case EntityEffect:
			break;
		case UnlockRecipes:
			break;
		case SelectAdvancementTab:
			break;
		case Advancements:
			break;
	}
	return nullptr;
}
