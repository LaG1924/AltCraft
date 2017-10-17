#include "Network.hpp"

#include <zlib.h>
#include <easylogging++.h>

#include "Socket.hpp"

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

std::shared_ptr<Packet> Network::ReceivePacket(ConnectionState state, bool useCompression) {
    if (useCompression) {
        int packetLength = stream->ReadVarInt();
        auto packetData = stream->ReadByteArray(packetLength);
        StreamBuffer streamBuffer(packetData.data(), packetData.size());
        
        int dataLength = streamBuffer.ReadVarInt();
        if (dataLength == 0) {
            auto packetData = streamBuffer.ReadByteArray(packetLength - streamBuffer.GetReadedLength());
            StreamBuffer streamBuffer(packetData.data(), packetData.size());
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

            StreamBuffer streamBuffer(uncompressedData.data(), uncompressedData.size());
            int packetId = streamBuffer.ReadVarInt();
            auto packet = ReceivePacketByPacketId(packetId, state, streamBuffer);
            return packet;            
        }
    } else {
        int packetSize = stream->ReadVarInt();
        auto packetData = stream->ReadByteArray(packetSize);
        StreamBuffer streamBuffer(packetData.data(), packetData.size());
        int packetId = streamBuffer.ReadVarInt();
        auto packet = ReceivePacketByPacketId(packetId, state, streamBuffer);
        return packet;
    }	
}

void Network::SendPacket(Packet &packet, int compressionThreshold) {
    if (compressionThreshold >= 0) {
        StreamCounter packetSize;
        packetSize.WriteVarInt(packet.GetPacketId());
        packetSize.WriteVarInt(0);
        packet.ToStream(&packetSize);
        if (packetSize.GetCountedSize() < compressionThreshold) {
            stream->WriteVarInt(packetSize.GetCountedSize());
            stream->WriteVarInt(0);
            stream->WriteVarInt(packet.GetPacketId());
            packet.ToStream(stream);
        } else {
            throw std::runtime_error("Compressing send data not supported");
        }
    }
    else {
        StreamCounter packetSize;
        packetSize.WriteVarInt(packet.GetPacketId());
        packet.ToStream(&packetSize);
        stream->WriteVarInt(packetSize.GetCountedSize());
        stream->WriteVarInt(packet.GetPacketId());
        packet.ToStream(stream);
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
