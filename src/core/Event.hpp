#pragma once

#include <queue>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <variant>
#include <functional>

#include <Vector.hpp>

enum class EventType {
	Echo,
	ChunkChanged,
};

struct EchoData {
	std::chrono::time_point<std::chrono::high_resolution_clock> time;
};

struct ChunkChangedData {
	Vector chunkPosition;
};

using EventData = std::variant<EchoData, ChunkChangedData>;

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

	void RegisterHandler(EventType type, HandlerFunc handler) {
		handlers[type] = handler;
	}

	void HandleEvent() {
		eventsMutex.lock();
		if (events.empty()) {
			eventsMutex.unlock();
			return;
		}
		Event event = events.front();
		events.pop();
		eventsMutex.unlock();
		auto function = handlers[event.type];
		function(event.data);
	}
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
	static void PushEvent(EventType type, EventData data) {
		if (!isStarted) {
			isStarted = true;
			std::thread(&EventAgregator::EventHandlingLoop).detach();
		}
		Event event;
		event.type = type;
		event.data = data;
		eventsToHandle.push(event);
	}
};