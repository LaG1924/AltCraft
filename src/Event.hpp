#pragma once

#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <variant>
#include <functional>

#include <SDL.h>

#include "Vector.hpp"
#include "Chat.hpp"

class Packet;

enum class EventType {
	Echo,
	ChunkChanged,
	ConnectToServer,
	ConnectionSuccessfull,
	Disconnect,
	RequestNetworkClient,
	RegisterNetworkClient,
	PlayerConnected,
	RemoveLoadingScreen,
	ConnectionFailed,
    Exit,
    Disconnected,
    Connecting,
    NetworkClientException,
    MouseMoved,
    KeyPressed,
    KeyReleased,
    InitalizeSectionRender,
    UpdateSectionsRender,
    CreateSectionRender,
    CreatedSectionRender,
    PlayerPosChanged,
    DeleteSectionRender,
    EntityChanged,
    NewRenderDataAvailable,
    BlockChange,
    RendererWorkerTask,
    ChunkDeleted,
    ChatMessageReceived,
    SendChatMessage,
    StateUpdated,
};

struct EchoData {
	std::chrono::time_point<std::chrono::high_resolution_clock> time;
};

struct ChunkChangedData {
	Vector chunkPosition;
};

struct ConnectToServerData {
    std::string username;
	std::string address;
	unsigned short port;
};

struct ConnectionSuccessfullData {
};

struct DisconnectData {
    std::string reason;
};

struct SendPacketData {
	std::shared_ptr<Packet> packet;
};

struct ReceivePacketData {
	std::shared_ptr<Packet> packet;
};

struct RequestNetworkClientData {

};

struct RegisterNetworkClientData {
};

struct PlayerConnectedData {
};

struct RemoveLoadingScreenData {

};

struct ConnectionFailedData {
	std::string reason;
};

struct ExitData {

};

struct DisconnectedData {
    std::string reason;
};

struct ConnectingData {

};

struct NetworkClientExceptionData {
    std::string what;
};

struct MouseMovedData {
    double x, y;
};

struct KeyPressedData {
    SDL_Scancode key;
};

struct KeyReleasedData {
    SDL_Scancode key;
};

struct InitalizeSectionRenderData {
    Vector pos;
};

struct CreateSectionRenderData {
    Vector pos;
};

struct CreatedSectionRenderData {
    Vector pos;
};

struct PlayerPosChangedData {
    VectorF newPos;
};

struct UpdateSectionsRenderData {

};

struct DeleteSectionRenderData {
    Vector pos;
};

struct EntityChangedData {
    unsigned int EntityId;
};

struct NewRenderDataAvailableData {

};

struct BlockChangeData {
    Vector SectionPos;
};

struct RendererWorkerTaskData {
    size_t WorkerId;
    Vector Task;
};

struct ChunkDeletedData {
    Vector pos;
};

struct ChatMessageReceivedData {
    Chat message;
    unsigned char position;
};

struct SendChatMessageData {
    std::string message;
};

struct StateUpdatedData {

};

using EventData = std::variant<EchoData, ChunkChangedData, ConnectToServerData, ConnectionSuccessfullData,
    DisconnectData, SendPacketData, ReceivePacketData, RequestNetworkClientData, RegisterNetworkClientData,
    PlayerConnectedData, RemoveLoadingScreenData, ConnectionFailedData, ExitData, DisconnectedData,
    ConnectingData, NetworkClientExceptionData, MouseMovedData, KeyPressedData, KeyReleasedData,
    InitalizeSectionRenderData, CreateSectionRenderData, CreatedSectionRenderData, PlayerPosChangedData,
    UpdateSectionsRenderData, DeleteSectionRenderData, EntityChangedData, NewRenderDataAvailableData,
    BlockChangeData, RendererWorkerTaskData, ChunkDeletedData, ChatMessageReceivedData,
    SendChatMessageData, StateUpdatedData>;

struct Event {
	EventType type;
	EventData data;
};

class EventListener {
	friend class EventAgregator;

	using HandlerFunc = std::function<void(EventData)>;

	std::map<EventType, HandlerFunc> handlers; //TODO: There must be more elegant solution than std::variant of all data

	std::mutex handlersMutex;

	std::queue<Event> events;

	std::mutex eventsMutex;

	void PushEvent(Event event);

    void DirectCall(Event event);

public:
	EventListener();
	~EventListener();
	bool IsEventsQueueIsNotEmpty();

	void RegisterHandler(EventType type, HandlerFunc handler);

	void HandleEvent();
};

class EventAgregator {
	friend EventListener;

	EventAgregator() = default;
	static std::queue<Event> eventsToHandle;
	static std::mutex queueMutex;
	static bool isStarted;
	static std::vector<EventListener *> listeners;
	static std::mutex listenersMutex;

	static void EventHandlingLoop();

	static void RegisterListener(EventListener &listener);
	static void UnregisterListener(EventListener &listener);

public:
	static void PushEvent(EventType type, EventData data);
    static void DirectEventCall(EventType, EventData data);
};