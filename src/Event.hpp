#pragma once

#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <variant>
#include <functional>

#include "Vector.hpp"
#include "Packet.hpp"

enum class EventType {
	Echo,
	ChunkChanged,
	ConnectToServer,
	ConnectionSuccessfull,
	GlobalAppState,
	Disconnect,
	SendPacket,
	ReceivePacket,
};

struct EchoData {
	std::chrono::time_point<std::chrono::high_resolution_clock> time;
};

struct ChunkChangedData {
	Vector chunkPosition;
};

struct ConnectToServerData {
	std::string address;
	unsigned short port;
};

struct ConnectionSuccessfullData {

};

enum class GlobalState {
	InitialLoading,
	MainMenu,
	Loading,
	InGame,
	PauseMenu,
	Exiting,
};

struct GlobalAppStateData {
	GlobalState state;
};

struct DisconnectData {

};

struct SendPacketData {
	std::shared_ptr<Packet> packet;
};

struct ReceivePacketData {
	std::shared_ptr<Packet> packet;
};

using EventData = std::variant<EchoData, ChunkChangedData, ConnectToServerData, ConnectionSuccessfullData,
		GlobalAppStateData, DisconnectData, SendPacketData, ReceivePacketData>;

struct Event {
	EventType type;
	EventData data;
};

class EventListener {
	friend class EventAgregator;

	using HandlerFunc = std::function<void(EventData)>;

	std::map<EventType, HandlerFunc> handlers; //TODO: There must be more elegant solution than std::variant of all data

	std::mutex eventsMutex;

	std::queue<Event> events;

	void PushEvent(Event event);

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
};