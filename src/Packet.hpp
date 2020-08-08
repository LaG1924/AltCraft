#pragma once

#include "Stream.hpp"

enum PacketNameLoginSB {
    LoginStart = 0x00,
    EncryptionResponse = 0x01,
};
enum PacketNamePlaySB {
    TeleportConfirm = 0x00,
    TabCompleteSB,
    ChatMessageSB,
    ClientStatus,
    ClientSettings,
    ConfirmTransactionSB,
    EnchantItem,
    ClickWindow,
    CloseWindowSB,
    PluginMessageSB,
    UseEntity,
    KeepAliveSB,
    Player,
    PlayerPosition,
    PlayerPositionAndLookSB,
    PlayerLook,
    VehicleMoveSB,
    SteerBoat,
    CraftRecipeRequest,
    PlayerAbilitiesSB,
    PlayerDigging,
    EntityAction,
    SteerVehicle,
    CraftingBookData,
    ResourcePackStatus,
    AdvancementTab,
    HeldItemChangeSB,
    CreativeInventoryAction,
    UpdateSign,
    AnimationSB,
    Spectate,
    PlayerBlockPlacement,
    UseItem,
};

enum PacketNameHandshakingCB {
    Handshake = 0x00,
};
enum PacketNameLoginCB {
    Disconnect = 0x00,
    EncryptionRequest = 0x01,
    LoginSuccess = 0x02,
    SetCompression = 0x03,
};
enum PacketNamePlayCB {
    SpawnObject = 0x00,
    SpawnExperienceOrb,
    SpawnGlobalEntity,
    SpawnMob,
    SpawnPainting,
    SpawnPlayer,
    AnimationCB,
    Statistics,
    BlockBreakAnimation,
    UpdateBlockEntity,
    BlockAction,
    BlockChange,
    BossBar,
    ServerDifficulty,
    TabCompleteCB,
    ChatMessageCB,
    MultiBlockChange,
    ConfirmTransactionCB,
    CloseWindowCB,
    OpenWindow,
    WindowItems,
    WindowProperty,
    SetSlot,
    SetCooldown,
    PluginMessageCB,
    NamedSoundEffect,
    DisconnectPlay,
    EntityStatus,
    Explosion,
    UnloadChunk,
    ChangeGameState,
    KeepAliveCB,
    ChunkData,
    Effect,
    Particle,
    JoinGame,
    Map,
    EntityRelativeMove,
    EntityLookAndRelativeMove,
    EntityLook,
    EntityCB,
    VehicleMove,
    OpenSignEditor,
    CraftRecipeResponse,
    PlayerAbilitiesCB,
    CombatEvent,
    PlayerListItem,
    PlayerPositionAndLookCB,
    UseBed,
    UnlockRecipes,
    DestroyEntities,
    RemoveEntityEffect,
    ResourcePackSend,
    Respawn,
    EntityHeadLook,
    SelectAdvancementTab,
    WorldBorder,
    Camera,
    HeldItemChangeCB,
    DisplayScoreboard,
    EntityMetadata,
    AttachEntity,
    EntityVelocity,
    EntityEquipment,
    SetExperience,
    UpdateHealth,
    ScoreboardObjective,
    SetPassengers,
    Teams,
    UpdateScore,
    SpawnPosition,
    TimeUpdate,
    Title,
    SoundEffect,
    PlayerListHeaderAndFooter,
    CollectItem,
    EntityTeleport,
    Advancements,
    EntityProperties,
    EntityEffect,
};

#define StringLen(str) (VarIntLen(str.size())+str.size())

struct Packet {
    virtual ~Packet() = default;
    virtual void ToStream(StreamOutput *stream) = 0;
    virtual void FromStream(StreamInput *stream) = 0;
	virtual uint32_t GetLen() const = 0;
    virtual int GetPacketId() = 0;

	constexpr static inline size_t VarIntLen(uint32_t i32){
		if (i32 & 0xF0000000)
			return 5;
		else if (i32 & 0x0FE00000)
			return 4;
		else if (i32 & 0x001FC000)
			return 3;
		else if (i32 & 0x00003F80)
			return 2;
		else
			return 1;
	}
//	constexpr static inline size_t VarLongLen(uint64_t i64){
//		size_t sz=0;
//		if (sz = VarIntLen((uint32_t)(i64 >> 32)))
//			return sz+5;
//		else
//			return VarIntLen((uint32_t) i64);
//	}
};

struct PacketHandshake : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteVarInt(protocolVersion);
        stream->WriteString(serverAddress);
        stream->WriteUShort(serverPort);
        stream->WriteVarInt(nextState);
    }

    void FromStream(StreamInput *stream) override {
        protocolVersion = stream->ReadVarInt();
        serverAddress = stream->ReadString();
        serverPort = stream->ReadUShort();
        nextState = stream->ReadVarInt();
    }

	uint32_t GetLen() const override {
		return VarIntLen(protocolVersion)
				+StringLen(serverAddress)
				+sizeof(serverPort)
				+VarIntLen(nextState);
	}

    int GetPacketId() override {
        return PacketNameHandshakingCB::Handshake;
    }

    int protocolVersion;
    std::string serverAddress;
    unsigned short serverPort;
    int nextState;
};

struct PacketLoginStart : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteString(Username);
    }

    void FromStream(StreamInput *stream) override {
        Username = stream->ReadString();
    }

	uint32_t GetLen() const override {
		return StringLen(Username);
	}

    int GetPacketId() override {
        return PacketNameLoginSB::LoginStart;
    }

    std::string Username;
};

struct PacketLoginSuccess : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteString(Uuid);
        stream->WriteString(Username);
    }

    void FromStream(StreamInput *stream) override {
        Uuid = stream->ReadString();
        Username = stream->ReadString();
    }

	uint32_t GetLen() const override {
		return StringLen(Username)
				+StringLen(Uuid);
	}

    int GetPacketId() override {
        return PacketNameLoginCB::LoginSuccess;
    }

    std::string Uuid;
    std::string Username;
};

struct PacketJoinGame : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteInt(EntityId);
        stream->WriteUByte(Gamemode);
        stream->WriteInt(Dimension);
        stream->WriteUByte(Difficulty);
        stream->WriteUByte(MaxPlayers);
        stream->WriteString(LevelType);
        stream->WriteBool(ReducedDebugInfo);
    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadInt();
        Gamemode = stream->ReadUByte();
        Dimension = stream->ReadInt();
        Difficulty = stream->ReadUByte();
        MaxPlayers = stream->ReadUByte();
        LevelType = stream->ReadString();
        ReducedDebugInfo = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return sizeof(EntityId)
				+sizeof(Gamemode)
				+sizeof(Dimension)
				+sizeof(Difficulty)
				+sizeof(MaxPlayers)
				+StringLen(LevelType)
				+sizeof(ReducedDebugInfo);
	}

    int GetPacketId() override {
        return PacketNamePlayCB::JoinGame;
    }

    int EntityId;
    unsigned char Gamemode;
    int Dimension;
    unsigned char Difficulty;
    unsigned char MaxPlayers;
    std::string LevelType;
    bool ReducedDebugInfo;
};

struct PacketDisconnectPlay : Packet {
    void ToStream(StreamOutput *stream) override {
		stream->WriteString(Reason);//TODO: Implement chat-wrapper
    }

    void FromStream(StreamInput *stream) override {
        Reason = stream->ReadChat().ToPlainText();
    }

	uint32_t GetLen() const override {
		return StringLen(Reason);
	}

    int GetPacketId() override {
        return PacketNamePlayCB::DisconnectPlay;
    }

    std::string Reason;
};

struct PacketSpawnPosition : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WritePosition(Location);
    }

    void FromStream(StreamInput *stream) override {
        Location = stream->ReadPosition();
    }

	uint32_t GetLen() const override {
		return 8; //Position
	}

    int GetPacketId() override {
        return PacketNamePlayCB::SpawnPosition;
    }

    Vector Location;
};

struct PacketKeepAliveCB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteLong(KeepAliveId);
    }

    void FromStream(StreamInput *stream) override {
        KeepAliveId = stream->ReadLong();
    }

	uint32_t GetLen() const override {
		return sizeof(KeepAliveId);
	}
    int GetPacketId() override {
        return PacketNamePlayCB::KeepAliveCB;
    }

    long long KeepAliveId;
};

struct PacketKeepAliveSB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteLong(KeepAliveId);
    }

    void FromStream(StreamInput *stream) override {
        KeepAliveId = stream->ReadLong();
    }

	uint32_t GetLen() const override {
		return sizeof(KeepAliveId);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::KeepAliveSB;
    }

    long long KeepAliveId;

    PacketKeepAliveSB(int KeepAliveId) : KeepAliveId(KeepAliveId) {}
};

struct PacketPlayerPositionAndLookCB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteDouble(X);
        stream->WriteDouble(Y);
        stream->WriteDouble(Z);
        stream->WriteFloat(Yaw);
        stream->WriteFloat(Pitch);
        stream->WriteUByte(Flags);
        stream->WriteVarInt(TeleportId);
    }

    void FromStream(StreamInput *stream) override {
        X = stream->ReadDouble();
        Y = stream->ReadDouble();
        Z = stream->ReadDouble();
        Yaw = stream->ReadFloat();
        Pitch = stream->ReadFloat();
        Flags = stream->ReadUByte();
        TeleportId = stream->ReadVarInt();
    }

	uint32_t GetLen() const override {
		return sizeof(X)
				+sizeof(Y)
				+sizeof(Z)
				+sizeof(Yaw)
				+sizeof(Pitch)
				+sizeof(Flags)
				+VarIntLen(TeleportId);
	}

    int GetPacketId() override {
        return PacketNamePlayCB::PlayerPositionAndLookCB;
    }

    double X;
    double Y;
    double Z;
    float Yaw;
    float Pitch;
    unsigned char Flags;
    int TeleportId;
};

struct PacketTeleportConfirm : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteVarInt(TeleportId);
    }

    void FromStream(StreamInput *stream) override {
        TeleportId = stream->ReadVarInt();
    }

	uint32_t GetLen() const override {
		return VarIntLen(TeleportId);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::TeleportConfirm;
    }

    int TeleportId;

    PacketTeleportConfirm(int TeleportId) : TeleportId(TeleportId) {}
};

struct PacketClientStatus : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteVarInt(ActionId);
    }

    void FromStream(StreamInput *stream) override {
        ActionId = stream->ReadVarInt();
    }

	uint32_t GetLen() const override {
		return VarIntLen(ActionId);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::ClientStatus;
    }

    int ActionId;

    PacketClientStatus(int ActionId) : ActionId(ActionId) {}
};

struct PacketPlayerPositionAndLookSB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteDouble(X);
        stream->WriteDouble(FeetY);
        stream->WriteDouble(Z);
        stream->WriteFloat(Yaw);
        stream->WriteFloat(Pitch);
        stream->WriteBool(OnGround);
    }

    void FromStream(StreamInput *stream) override {
        X = stream->ReadDouble();
        FeetY = stream->ReadDouble();
        Z = stream->ReadDouble();
        Yaw = stream->ReadFloat();
        Pitch = stream->ReadFloat();
        OnGround = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return sizeof(X)
				+sizeof(FeetY)
				+sizeof(Z)
				+sizeof(Yaw)
				+sizeof(Pitch)
				+sizeof(OnGround);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::PlayerPositionAndLookSB;
    }


    double X;
    double FeetY;
    double Z;
    float Yaw;
    float Pitch;
    bool OnGround;

    PacketPlayerPositionAndLookSB(double X, double FeetY, double Z,
                                  float Yaw, float Pitch, bool OnGround) : X(X), FeetY(FeetY), Z(Z), Yaw(Yaw),
                                                                           Pitch(Pitch), OnGround(OnGround) {}
};

struct PacketChunkData : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteInt(ChunkX);
        stream->WriteInt(ChunkZ);
        stream->WriteBool(GroundUpContinuous);
        stream->WriteInt(PrimaryBitMask);
        stream->WriteVarInt(Data.size());
        stream->WriteByteArray(Data);
        stream->WriteVarInt(BlockEntities.size());
        //LOG(FATAL) << "Serializing unimplemented packet";
    }

    void FromStream(StreamInput *stream) override {
        ChunkX = stream->ReadInt();
        ChunkZ = stream->ReadInt();
        GroundUpContinuous = stream->ReadBool();
        PrimaryBitMask = stream->ReadVarInt();
        int Size = stream->ReadVarInt();
        Data = stream->ReadByteArray(Size);
        int NumberOfBlockEntities = stream->ReadVarInt(); //TODO: Need NBT
        for (int i = 0; i < NumberOfBlockEntities; i++) {
            //BlockEntities[i] = stream->ReadNbt();
        }
    }

	uint32_t GetLen() const override {
		return sizeof(ChunkX)
				+sizeof(ChunkZ)
				+sizeof(GroundUpContinuous)
				+sizeof(PrimaryBitMask)
				+StringLen(Data)
				+StringLen(BlockEntities);
	}

    int GetPacketId() override {
        return PacketNamePlayCB::ChunkData;
    }

    int ChunkX;
    int ChunkZ;
    bool GroundUpContinuous;
    int PrimaryBitMask;
    //int Size;
    std::vector<unsigned char> Data;
    //int NumberOfBlockEntities;
    std::vector<int> BlockEntities; //TODO: Replace int with NbtTag and implement NbtTree
};

struct PacketPlayerPosition : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteDouble(X);
        stream->WriteDouble(FeetY);
        stream->WriteDouble(Z);
        stream->WriteBool(OnGround);
    }

    void FromStream(StreamInput *stream) override {
        X = stream->ReadDouble();
        FeetY = stream->ReadDouble();
        Z = stream->ReadDouble();
        OnGround = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return sizeof(X)
				+sizeof(FeetY)
				+sizeof(Z)
				+sizeof(OnGround);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::PlayerPosition;
    }

    double X;
    double FeetY;
    double Z;
    bool OnGround;

    PacketPlayerPosition(double X, double Y, double Z, bool ground) : X(X), FeetY(Y), Z(Z), OnGround(ground) {}
};

struct PacketPlayerLook : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteFloat(Yaw);
        stream->WriteFloat(Pitch);
        stream->WriteBool(OnGround);
    }

    void FromStream(StreamInput *stream) override {
        Yaw = stream->ReadFloat();
        Pitch = stream->ReadFloat();
        OnGround = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return sizeof(Yaw)
				+sizeof(Pitch)
				+sizeof(OnGround);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::PlayerLook;
    }

    float Yaw;
    float Pitch;
    bool OnGround;

    PacketPlayerLook(float Yaw, float Pitch, bool ground) : Yaw(Yaw), Pitch(Pitch), OnGround(ground) {}
};

struct PacketUpdateHealth : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteFloat(Health);
        stream->WriteVarInt(Food);
        stream->WriteFloat(FoodSaturation);
    }

    void FromStream(StreamInput *stream) override {
        Health = stream->ReadFloat();
        Food = stream->ReadVarInt();
        FoodSaturation = stream->ReadFloat();
    }

	uint32_t GetLen() const override {
		return sizeof(Health)
				+VarIntLen(Food)
				+sizeof(FoodSaturation);
	}

    int GetPacketId() override {
        return PacketNamePlayCB::UpdateHealth;
    }

    float Health;
    int Food;
    float FoodSaturation;
};

struct PacketSpawnObject : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        ObjectUuid = stream->ReadUuid();
        Type = stream->ReadByte();
        X = stream->ReadDouble();
        Y = stream->ReadDouble();
        Z = stream->ReadDouble();
        Pitch = stream->ReadAngle();
        Yaw = stream->ReadAngle();
        Data = stream->ReadInt();
        VelocityX = stream->ReadShort();
        VelocityY = stream->ReadShort();
        VelocityZ = stream->ReadShort();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::SpawnObject;
    }

    int EntityId;
    Uuid ObjectUuid;
    unsigned char Type;
    double X, Y, Z;
    unsigned char Pitch, Yaw;
    int Data;
    short VelocityX;
    short VelocityY;
    short VelocityZ;
};

struct PacketEntityRelativeMove : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        DeltaX = stream->ReadShort();
        DeltaY = stream->ReadShort();
        DeltaZ = stream->ReadShort();
        OnGround = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::EntityRelativeMove;
    }

    int EntityId;
    short DeltaX, DeltaY, DeltaZ;
    bool OnGround;
};

struct PacketEntityLookAndRelativeMove : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        DeltaX = stream->ReadShort();
        DeltaY = stream->ReadShort();
        DeltaZ = stream->ReadShort();
        //TODO: WTF?
        /*Yaw = stream->ReadAngle();
        Pitch = stream->ReadAngle();
        OnGround = stream->ReadBool();*/
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::EntityLookAndRelativeMove;
    }

    int EntityId;
    short DeltaX, DeltaY, DeltaZ;
    unsigned char Yaw, Pitch;
    bool OnGround;
};

struct PacketEntityLook : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        Yaw = stream->ReadAngle();
        Pitch = stream->ReadAngle();
        OnGround = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::EntityLook;
    }

    int EntityId;
    unsigned char Yaw, Pitch;
    bool OnGround;
};

struct PacketEntityVelocity : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        VelocityX = stream->ReadShort();
        VelocityY = stream->ReadShort();
        VelocityZ = stream->ReadShort();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::EntityVelocity;
    }

    int EntityId;
    short VelocityX;
    short VelocityY;
    short VelocityZ;
};

struct PacketEntityTeleport : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        X = stream->ReadDouble();
        Y = stream->ReadDouble();
        Z = stream->ReadDouble();
        Yaw = stream->ReadAngle();
        Pitch = stream->ReadAngle();
        OnGround = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::EntityTeleport;
    }

    int EntityId;
    double X, Y, Z;
    unsigned char Yaw, Pitch;
    bool OnGround;
};

struct PacketSpawnPlayer : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        PlayerUuid = stream->ReadUuid();
        X = stream->ReadDouble();
        Y = stream->ReadDouble();
        Z = stream->ReadDouble();
        Yaw = stream->ReadAngle();
        Pitch = stream->ReadAngle();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::SpawnPlayer;
    }

    int EntityId;
    Uuid PlayerUuid;
    double X, Y, Z;
    unsigned char Yaw, Pitch;


};

struct PacketDestroyEntities : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        int count = stream->ReadVarInt();
        EntityIds.reserve(count);
        for (int i = 0; i < count; i++) {
            int entityId = stream->ReadVarInt();
            EntityIds.push_back(entityId);
        }
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::DestroyEntities;
    }

    std::vector <unsigned int> EntityIds;
};

struct PacketSpawnMob : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        EntityId = stream->ReadVarInt();
        EntityUuid = stream->ReadUuid();
        type = stream->ReadVarInt();
        X = stream->ReadDouble();
        Y = stream->ReadDouble();
        Z = stream->ReadDouble();
        Yaw = stream->ReadAngle();
        Pitch = stream->ReadAngle();
        HeadPitch = stream->ReadAngle();
        VelocityX = stream->ReadShort();
        VelocityY = stream->ReadShort();
        VelocityZ = stream->ReadShort();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::SpawnMob;
    }

    unsigned int EntityId;
    Uuid EntityUuid;
    int type;
    double X, Y, Z;
    unsigned char Yaw, Pitch, HeadPitch;
    short VelocityX, VelocityY, VelocityZ;
    //Metadata
};

struct PacketBlockChange : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        Position = stream->ReadPosition();
        BlockId = stream->ReadVarInt();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::BlockChange;
    }

    Vector Position;
    int BlockId;
};

struct PacketMultiBlockChange : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        ChunkX = stream->ReadInt();
        ChunkZ = stream->ReadInt();
        int recordCount = stream->ReadVarInt();
        for (int i = 0; i < recordCount; i++) {
            Record record;
            record.HorizontalPosition = stream->ReadUByte();
            record.YCoordinate = stream->ReadUByte();
            record.BlockId = stream->ReadVarInt();
            Records.push_back(record);
        }
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::MultiBlockChange;
    }

    int ChunkX;
    int ChunkZ;
    struct Record {
        unsigned char HorizontalPosition;
        unsigned char YCoordinate;
        int BlockId;
    };
    std::vector<Record> Records;
};

struct PacketTimeUpdate : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WorldAge = stream->ReadLong();
        TimeOfDay = stream->ReadLong();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::TimeUpdate;
    }

    long long WorldAge;
    long long TimeOfDay;
};

struct PacketUnloadChunk : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        ChunkX = stream->ReadInt();
        ChunkZ = stream->ReadInt();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::UnloadChunk;
    }

    int ChunkX;
    int ChunkZ;
};

struct PacketCloseWindowCB : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadUByte();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::CloseWindowCB;
    }

    unsigned char WindowId;
};

struct PacketOpenWindow : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadUByte();
        WindowType = stream->ReadString();
        WindowTitle = stream->ReadChat().ToPlainText();
        NumberOfSlots = stream->ReadUByte();

        if (WindowType == "EntityHorse")
            EntityId = stream->ReadInt();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::OpenWindow;
    }

    unsigned char WindowId;
    std::string WindowType;
    std::string WindowTitle;
    unsigned char NumberOfSlots;
    int EntityId;
};

struct PacketWindowItems : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadUByte();
        short count = stream->ReadShort();
        for (int i = 0; i < count; i++)
            SlotData.push_back(stream->ReadSlot());
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::WindowItems;
    }

    unsigned char WindowId;
    std::vector<SlotDataType> SlotData;
};

struct PacketWindowProperty : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadUByte();
        Property = stream->ReadShort();
        Value = stream->ReadShort();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::WindowProperty;
    }

    unsigned char WindowId;
    short Property;
    short Value;
};

struct PacketSetSlot : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadByte();
        Slot = stream->ReadShort();
        SlotData = stream->ReadSlot();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::SetSlot;
    }

    signed char WindowId;
    short Slot;
    SlotDataType SlotData;
};

struct PacketConfirmTransactionCB : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadByte();
        ActionNumber = stream->ReadShort();
        Accepted = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::ConfirmTransactionCB;
    }

    signed char WindowId;
    short ActionNumber;
    bool Accepted;
};

struct PacketConfirmTransactionSB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteByte(WindowId);
        stream->WriteShort(ActionNumber);
        stream->WriteBool(Accepted);
    }

    void FromStream(StreamInput *stream) override {
        WindowId = stream->ReadByte();
        ActionNumber = stream->ReadShort();
        Accepted = stream->ReadBool();
    }

	uint32_t GetLen() const override {
		return sizeof(WindowId)
				+sizeof(ActionNumber)
				+sizeof(Accepted);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::ConfirmTransactionSB;
    }

    signed char WindowId;
    short ActionNumber;
    bool Accepted;
};

struct PacketClickWindow : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteUByte(WindowId);
        stream->WriteShort(Slot);
        stream->WriteByte(Button);
        stream->WriteShort(ActionNumber);
        stream->WriteVarInt(Mode);
        stream->WriteSlot(ClickedItem);
    }

    void FromStream(StreamInput *stream) override {

    }

	uint32_t GetLen() const override {
		return sizeof(WindowId)
				+sizeof(Slot)
				+sizeof(Button)
				+sizeof(ActionNumber)
				+VarIntLen(Mode)
				+sizeof(ClickedItem.BlockId)
				+sizeof(ClickedItem.ItemCount)
				+sizeof(ClickedItem.ItemDamage);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::ClickWindow;
    }

    unsigned char WindowId;
    short Slot;
    signed char Button;
    short ActionNumber;
    int Mode;
    SlotDataType ClickedItem;

    PacketClickWindow(unsigned char windowId, short slot, signed char button, short actionNumber, int mode, SlotDataType ClickedItem) : WindowId(windowId), Slot(slot),
        Button(button), ActionNumber(actionNumber), Mode(mode), ClickedItem(ClickedItem) {};
};

struct PacketCloseWindowSB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteUByte(WindowId);
    }

    void FromStream(StreamInput *stream) override {

    }

	uint32_t GetLen() const override {
		return sizeof(WindowId);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::CloseWindowSB;
    }

    unsigned char WindowId;
};

struct PacketDisconnect : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        Reason = stream->ReadChat().ToPlainText();
    }

	uint32_t GetLen() const override {
		return StringLen(Reason);
	}

    int GetPacketId() override {
        return PacketNameLoginCB::Disconnect;
    }

    std::string Reason;
};

struct PacketSetCompression : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        Threshold = stream->ReadVarInt();
    }

	uint32_t GetLen() const override {
		return sizeof(Threshold);
	}

    int GetPacketId() override {
        return PacketNameLoginCB::SetCompression;
    }

    int Threshold;
};

struct PacketChatMessageCB : Packet {
    void ToStream(StreamOutput *stream) override {

    }

    void FromStream(StreamInput *stream) override {
        JsonData = stream->ReadChat();
        Position = stream->ReadByte();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::ChatMessageCB;
    }

    Chat JsonData;
    unsigned char Position;
};

struct PacketChatMessageSB : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteString(Message);
    }

    void FromStream(StreamInput *stream) override {

    }

	uint32_t GetLen() const override {
		return StringLen(Message);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::ChatMessageSB;
    }

    std::string Message;

    PacketChatMessageSB(const std::string msg) : Message(msg) {};
};

struct PacketPlayerDigging : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WriteVarInt(Status);
        stream->WritePosition(Location);
        stream->WriteByte(Face);
    }

    void FromStream(StreamInput *stream) override {

    }

	uint32_t GetLen() const override {
		return VarIntLen(Status)
				+8
				+sizeof(Face);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::PlayerDigging;
    }

    int Status;
    Vector Location;
    signed char Face;

    PacketPlayerDigging(int status, const Vector& location, signed char face) : Status(status),Location(location),Face(face) {};
};

struct PacketPlayerBlockPlacement : Packet {
    void ToStream(StreamOutput *stream) override {
        stream->WritePosition(location);
        stream->WriteByte(face);
        stream->WriteByte(hand);
        stream->WriteFloat(cursorPositionX);
        stream->WriteFloat(cursorPositionY);
        stream->WriteFloat(cursorPositionZ);
    }

    void FromStream(StreamInput *stream) override {

    }

	uint32_t GetLen() const override {
		return 8
				+sizeof(face)
				+sizeof(hand)
				+sizeof(cursorPositionX)
				+sizeof(cursorPositionY)
				+sizeof(cursorPositionZ);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::PlayerBlockPlacement;
    }

    PacketPlayerBlockPlacement(
        const Vector& location, signed char face, unsigned char hand,
        float cursorPositionX, float cursorPositionY, float cursorPositionZ)
            : location(location), face(face), hand(hand), cursorPositionX(cursorPositionX),
              cursorPositionY(cursorPositionY), cursorPositionZ(cursorPositionZ) {};

    Vector location;
    signed char face;
    unsigned char hand;
    float cursorPositionX;
    float cursorPositionY;
    float cursorPositionZ;
};

struct PacketRespawn : Packet {
    void ToStream(StreamOutput* stream) override {

    }

    void FromStream(StreamInput* stream) override {
        Dimension = stream->ReadInt();
        Difficulty = stream->ReadUByte();
        Gamemode = stream->ReadUByte();
        LevelType = stream->ReadString();
    }

	uint32_t GetLen() const override {
		return 0;
	}

    int GetPacketId() override {
        return PacketNamePlayCB::Respawn;
    }

    int Dimension;
    unsigned char Difficulty;
    unsigned char Gamemode;
    std::string LevelType;
};

struct PacketPluginMessageSB : Packet {
    void ToStream(StreamOutput* stream) override {
        stream->WriteString(Channel);
        stream->WriteByteArray(Data);
    }

    void FromStream(StreamInput* stream) override {

    }

	uint32_t GetLen() const override {
		return StringLen(Channel)
				+Data.size();
	}

    int GetPacketId() override {
        return PacketNamePlaySB::PluginMessageSB;
    }

    PacketPluginMessageSB(const std::string& channel, const std::vector<unsigned char> data) : Channel(channel), Data(data) {}

    std::string Channel;
    std::vector<unsigned char> Data;
};

struct PacketClientSettings : Packet {
    void ToStream(StreamOutput* stream) override {
        stream->WriteString(Locale);
        stream->WriteByte(ViewDistance);
        stream->WriteVarInt(ChatMode);
        stream->WriteBool(ChatColors);
        stream->WriteUByte(DisplayedSkinParts);
        stream->WriteVarInt(MainHand);
    }

    void FromStream(StreamInput* stream) override {

    }

	uint32_t GetLen() const override {
		return StringLen(Locale)
				+sizeof(ViewDistance)
				+VarIntLen(ChatMode)
				+sizeof(ChatColors)
				+sizeof(DisplayedSkinParts)
				+VarIntLen(MainHand);
	}

    int GetPacketId() override {
        return PacketNamePlaySB::ClientSettings;
    }

    PacketClientSettings(
        const std::string locale,
        unsigned char viewDistance,
        int chatMode,
        bool chatColors,
        unsigned char displayedSkinParts,
        int mainHand) :
        Locale(locale),
        ViewDistance(viewDistance),
        ChatMode(chatMode),
        ChatColors(chatColors),
        DisplayedSkinParts(displayedSkinParts),
        MainHand(mainHand) {}

    std::string Locale;
    unsigned char ViewDistance;
    int ChatMode;
    bool ChatColors;
    unsigned char DisplayedSkinParts;
    int MainHand;
};

