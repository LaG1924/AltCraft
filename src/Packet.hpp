#pragma once

#include <easylogging++.h>

#include "Stream.hpp"

enum PacketNameLoginSB {
	LoginStart = 0x00,
	EncryptionResponse = 0x01,
};
enum PacketNamePlaySB {
	TeleportConfirm,
	PrepareCraftingGrid,
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
	Entity,
	VehicleMove,
	OpenSignEditor,
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

struct Packet {
	virtual ~Packet() = default;
	virtual void ToStream(StreamOutput *stream) = 0;
	virtual void FromStream(StreamInput *stream) = 0;
	virtual int GetPacketId() = 0;
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
		stream->WriteString(Reason); //TODO: Implement chat-wrapper
	}

	void FromStream(StreamInput *stream) override {
		Reason = stream->ReadChat();
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

	int GetPacketId() override {
		return PacketNamePlayCB::SpawnPosition;
	}

	Vector Location;
};

struct PacketKeepAliveCB : Packet {
	void ToStream(StreamOutput *stream) override {
		stream->WriteVarInt(KeepAliveId);
	}

	void FromStream(StreamInput *stream) override {
		KeepAliveId = stream->ReadVarInt();
	}

	int GetPacketId() override {
		return PacketNamePlayCB::KeepAliveCB;
	}

	int KeepAliveId;
};

struct PacketKeepAliveSB : Packet {
	void ToStream(StreamOutput *stream) override {
		stream->WriteVarInt(KeepAliveId);
	}

	void FromStream(StreamInput *stream) override {
		KeepAliveId = stream->ReadVarInt();
	}

	int GetPacketId() override {
		return PacketNamePlaySB::KeepAliveSB;
	}

	int KeepAliveId;

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
		LOG(FATAL) << "Serializing unimplemented packet";
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

	int GetPacketId() override {
		return PacketNamePlayCB::UpdateHealth;
	}

	float Health;
	int Food;
	float FoodSaturation;
};