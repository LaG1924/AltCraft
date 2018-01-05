#pragma once

#include <typeinfo>
#include <memory>
#include <functional>
#include <queue>
#include <map>
#include <list>
#include <mutex>


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
			throw std::runtime_error(std::string("Type ") + typeid(T).name() + " encountered but " + data->Type().name() + " expected");
		return static_cast<EventData<T>*>(data.get())->data;
	}
};

class EventListener {
	friend class EventSystem;
	using HandlerType = std::function<void(const Event&)>;
	std::queue<Event> events;
	std::map<size_t, HandlerType> handlers;
	std::recursive_mutex mutex;
public:
	EventListener();

	~EventListener();

	void HandleEvent();

	void HandleAllEvents();

	bool NotEmpty();

	void WaitEvent();

	void RegisterHandler(size_t eventId, const HandlerType &data) {
		mutex.lock();
		handlers[eventId] = data;
        mutex.unlock();
	}

	void RegisterHandler(const char *eventId, const HandlerType & data) {
		RegisterHandler(StrHash(eventId), data);
	}
};

class EventSystem {
	friend class EventListener;
	static std::list<EventListener*> listeners;
	static std::recursive_mutex listenersMutex;

public:
	template <typename T>
	static void PushEvent(size_t eventId, T data) {
		Event event(eventId, data);

		for (auto& listener : listeners) {
			//if (!listener->mutex.try_lock()) throw std::runtime_error("WHY?!");
			listener->mutex.lock();
			auto it = listener->handlers.find(eventId);
			if (it == listener->handlers.end()) {
				listener->mutex.unlock();
				continue;
			}

			listener->events.push(event);

			listener->mutex.unlock();
		}
	}

	template <typename T>
	static void DirectEventCall(size_t eventId, T data) {
		Event event(eventId, data);

		listenersMutex.lock();
		for (auto & listener : listeners) {
			listener->mutex.lock();
			auto it = listener->handlers.find(eventId);
			if (it == listener->handlers.end())
				continue;			

			it->second(event);
			listener->mutex.unlock();
		}
		listenersMutex.unlock();
	}
};

#define PUSH_EVENT(eventName, data) EventSystem::PushEvent(StrHash(eventName),data)

#define DIRECT_EVENT_CALL(eventName,data) EventSystem::DirectEventCall(StrHash(eventName),data)