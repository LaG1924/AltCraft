#pragma once

#include <vector>
#include "Field.hpp"
#include "FieldParser.hpp"

enum ConnectionState {
    Login,
    Handshaking,
    Play,
    Status,
};

enum PacketsClientBound{
    SpawnObject=0x00,
    SpawnExperienceOrb,
    SpawnGlobalEntity,
    SpawnMob,
    SpawnPainting,
    SpawnPlayer,
    Animation,
    Statistics,
    BlockBreakAnimation,
    UpdateBlockEntity,
    BlockAction,
    BlockChange,
    BossBar,
    ServerDifficulty,
    Tab,
    ChatMessage,
    MultiBlockChange,
    ConfirmTransaction,
    CloseWindow,
    OpenWindow,
    WindowItems,
    WindowProperty,
    SetSlot,
    SetCooldown,
    PluginMessage,
    NamedSoundEffect,
    Disconnect,
    EntityStatus,
    Explosion,
    UnloadChunk,
    ChangeGameState,
    KeepAlive,
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
    PlayerAbilities,
    CombatEvent,
    PlayerListItem,
    PlayerPositionAndLook,
    UseBed,
    DestroyEntities,
    RemoveEntityEffect,
    ResourcePackSend,
    Respawn,
    EntityHeadLook,
    WorldBorder,
    Camera,
    HeldItemChange,
    DisplayScoreboard,
    EntityMetadata_,
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
    EntityProperties,
    EntityEffect,
};

class Packet {
public:
    Packet(int id);

    Packet(byte *data);

    Packet(const Packet &other);

    ~Packet() {
        delete[] m_data;
    }

    int GetId();

    void AddField(Field field);

    void ParseField(FieldType type, size_t len = 0);

    void ParseFieldArray(Field &field, FieldType type, size_t len);

    Field & GetField(int id);

    size_t GetLength();

    void CopyToBuff(byte *ptr);

    void swap(Packet &other);

    Packet operator=(Packet other);

private:
    Packet();

    std::vector<Field> m_fields;
    byte *m_data = nullptr;
    byte *m_parsePtr = nullptr;
    size_t m_dataLength = 0;
};