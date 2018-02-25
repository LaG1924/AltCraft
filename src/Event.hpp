#pragma once

#include <typeinfo>
#include <memory>
#include <functional>
#include <queue>
#include <map>
#include <list>
#include <mutex>

#include <easylogging++.h>


size_t constexpr StrHash(char const *input) {
	return *input ? static_cast<size_t>(*input) + 33 * StrHash(input + 1) : 5381;
}

class Event {
	struct EventDataBase {
		virtual ~EventDataBase() {}
		virtual const std::type_info& Type() const = 0;
	};

	template<typename T>
	struct EventData : EventDataBase {
		EventData(const T &val) : data(val) {}

		const std::type_info& Type() const override {
			return typeid(data);
		}

		T data;
	};

	std::shared_ptr<EventDataBase> data;

public:
	const size_t id;

	template<typename T>
	Event(size_t eventId, const T &value) :
		id(eventId),
		data(std::make_shared<EventData<T>>(value)) {}

	~Event() = default;

	Event(const Event &other) = default;

	Event &operator=(const Event &) = default;

	Event(Event &&) = delete;

	Event &operator=(Event &&) = delete;

	template<typename T>
	const T& get() const {
		if (typeid(T) != data->Type())
			throw std::runtime_error(std::string("Type ") + typeid(T).name() +" encountered but " + data->Type().name() + " expected");
		return static_cast<EventData<T>*>(data.get())->data;
	}
};

class EventListener {
	friend class EventSystem;
	using HandlerType = std::function<void(const Event&)>;
	std::map<size_t, HandlerType> handlers;
	std::recursive_mutex handlersMutex;
	std::queue<Event> events;
	std::recursive_mutex eventsMutex;
	std::queue<Event> rawEvents;
	std::recursive_mutex rawEventsMutex;
	
public:
	EventListener();

	~EventListener();

	void HandleEvent();

	void HandleAllEvents();

	bool NotEmpty();

	void RegisterHandler(size_t eventId, const HandlerType &data);

	void RegisterHandler(const char *eventId, const HandlerType & data) {
		RegisterHandler(StrHash(eventId), data);
	}

	void PollEvents();
};

class EventSystem {
	friend class EventListener;
	static std::list<EventListener*> listeners;
	static std::recursive_mutex listenersMutex;

public:
	template <typename T>
	static void PushEvent(size_t eventId, T data) {
		Event event(eventId, data);

		std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
		for (auto& listener : listeners) {
			std::lock_guard<std::recursive_mutex> rawEventLock (listener->rawEventsMutex);
			listener->rawEvents.push(event);
		}
	}

	template <typename T>
	static void DirectEventCall(size_t eventId, T data) {
		Event event(eventId, data);
		std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
		for (auto & listener : listeners) {
			std::lock_guard<std::recursive_mutex> handlersLock (listener->handlersMutex);
			auto it = listener->handlers.find(eventId);
			if (it == listener->handlers.end())
				continue;

			it->second(event);
		}
	}
};

#define PUSH_EVENT(eventName, data) EventSystem::PushEvent(StrHash(eventName),data) //; LOG(INFO)<<"PUSH_EVENT "<<eventName;

#define DIRECT_EVENT_CALL(eventName,data) EventSystem::DirectEventCall(StrHash(eventName),data) //; LOG(INFO)<<"DIRECT_CALL "<<eventName;