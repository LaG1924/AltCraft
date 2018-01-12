#include "Event.hpp"

std::list<EventListener*> EventSystem::listeners;
std::recursive_mutex EventSystem::listenersMutex;

EventListener::EventListener() {
	std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
	EventSystem::listeners.push_back(this);
}

EventListener::~EventListener() {
	std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
	EventSystem::listeners.remove(this);
}

void EventListener::HandleEvent() {
	if (!NotEmpty())
		return;

	std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
	Event event = events.front();
	events.pop();	
	if (handlers[event.id]) {
		handlers[event.id](event);
	}
}

void EventListener::HandleAllEvents() {
    if (!NotEmpty())
        return;

	std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
	while (!events.empty()) {
		Event event = events.front();
		events.pop();
		if (handlers[event.id]) {
			handlers[event.id](event);
		}
	}
}

bool EventListener::NotEmpty() {
	bool ret = !events.empty();
	return ret;
}

void EventListener::WaitEvent() {
	std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
	while (events.empty()) {
		mutex.unlock();
		mutex.lock();
	}
}

void EventListener::RegisterHandler(size_t eventId, const EventListener::HandlerType &data) {
	std::lock_guard<std::recursive_mutex> lock(EventSystem::listenersMutex);
	handlers[eventId] = data;
}
