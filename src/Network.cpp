#include "Network.hpp"

#include <zlib.h>
#include <easylogging++.h>

Network::Network(std::string address, unsigned short port) {
	try {
		socket = std::make_unique<Socket>(address, port);
	} catch (std::exception &e) {
		LOG(WARNING) << "Connection failed: " << e.what();
		throw;
	}
}

uint32_t Network::ReadPacketLen(){
	unsigned char data[5] = {0};
	size_t dataLen = 0;

	do {
		socket->ReadData(&data[dataLen], 1);
	} while ((data[dataLen++] & 0x80) != 0);

	unsigned int parsed = 0;
	uint32_t result = 0;
	char read;
	do {
		assert(parsed<5);
		read = data[parsed];
		int value = (read & 0b01111111);
		result |= (value << (7 * parsed));
		parsed++;
	} while ((read & 0b10000000) != 0);

	return result;
}

std::shared_ptr<Packet> Network::ReceivePacket(ConnectionState state, bool useCompression) {
	size_t packetLength = ReadPacketLen();
	StreamROBuffer streamBuffer(packetLength);
	socket->ReadData(streamBuffer.buffer, packetLength);
    if (useCompression) {
		int dataLength = streamBuffer.ReadVarInt();
		if (dataLength == 0) {
            int packetId = streamBuffer.ReadVarInt();
            auto packet = ReceivePacketByPacketId(packetId, state, streamBuffer);
            return packet;
        } else {
            std::vector<unsigned char> compressedData = streamBuffer.ReadByteArray(packetLength - streamBuffer.GetReadedLength());
            std::vector<unsigned char> uncompressedData;
            uncompressedData.resize(dataLength);

            z_stream stream;
            stream.avail_in = compressedData.size();
            stream.next_in = compressedData.data();
            stream.avail_out = uncompressedData.size();
            stream.next_out = uncompressedData.data();
            stream.zalloc = Z_NULL;
            stream.zfree = Z_NULL;
            stream.opaque = Z_NULL;
            if (inflateInit(&stream) != Z_OK)
                throw std::runtime_error("Zlib decompression initalization error");

            int status = inflate(&stream, Z_FINISH);
            switch (status) {
            case Z_STREAM_END:
                break;
            case Z_OK:                            
            case Z_STREAM_ERROR:
            case Z_BUF_ERROR:
                throw std::runtime_error("Zlib decompression error: " + std::to_string(status));
            }

            if (inflateEnd(&stream) != Z_OK)
                throw std::runtime_error("Zlib decompression end error");

			StreamROBuffer uncompressedStreamBuffer(uncompressedData.data(), uncompressedData.size());
			int packetId = uncompressedStreamBuffer.ReadVarInt();
			auto packet = ReceivePacketByPacketId(packetId, state, uncompressedStreamBuffer);
            return packet;            
        }
    } else {
        int packetId = streamBuffer.ReadVarInt();
        auto packet = ReceivePacketByPacketId(packetId, state, streamBuffer);
        return packet;
    }	
}

void Network::Connect(unsigned char *buffPtr, size_t buffLen) {
	socket->Connect(buffPtr, buffLen);
}

void Network::SendPacket(Packet &packet, int compressionThreshold, bool more) {
	uint32_t len = packet.GetLen() + VarIntLen(packet.GetPacketId());
	if (compressionThreshold >= 0) {
//		FIXME: implement packet compression
//		if (len < compressionThreshold)
		StreamWOBuffer buffer((len + VarIntLen(0))+VarIntLen(len + VarIntLen(0)));
		buffer.WriteVarInt(len + VarIntLen(0));
		buffer.WriteVarInt(0);
		buffer.WriteVarInt(packet.GetPacketId());
		packet.ToStream(&buffer);
		socket->SendData(buffer.buffer, buffer.size, more);
	} else {
		StreamWOBuffer buffer(len+VarIntLen(len));
		buffer.WriteVarInt(len);
		buffer.WriteVarInt(packet.GetPacketId());
		packet.ToStream(&buffer);
		socket->SendData(buffer.buffer, buffer.size, more);
	}
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
            break;
		case Login:
            switch (packetId) {
            case PacketNameLoginCB::LoginSuccess:
                packet = std::make_shared<PacketLoginSuccess>();
                break;
            case PacketNameLoginCB::SetCompression:
                packet = std::make_shared<PacketSetCompression>();
                break;
            case PacketNameLoginCB::Disconnect:
                packet = std::make_shared<PacketDisconnect>();
                break;
            }
			break;
		case Play:
			packet = ParsePacketPlay(static_cast<PacketNamePlayCB>(packetId));
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
            return std::make_shared<PacketChatMessageCB>();
		case MultiBlockChange:
            return std::make_shared<PacketMultiBlockChange>();
		case ConfirmTransactionCB:
            return std::make_shared<PacketConfirmTransactionCB>();
		case CloseWindowCB:
            return std::make_shared<PacketCloseWindowCB>();
		case OpenWindow:
            return std::make_shared<PacketOpenWindow>();
		case WindowItems:
            return std::make_shared<PacketWindowItems>();
		case WindowProperty:
            return std::make_shared<PacketWindowProperty>();
		case SetSlot:
            return std::make_shared<PacketSetSlot>();
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
		case CraftRecipeResponse:
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
			return std::make_shared<PacketRespawn>();
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
