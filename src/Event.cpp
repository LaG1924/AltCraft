#include "Event.hpp"

#include <optick.h>

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
	OPTICK_EVENT();
	if (Empty())
		return;

	std::lock_guard<std::recursive_mutex> eventsLock (eventsMutex);
	Event event = events.front();
	events.pop();
	std::lock_guard<std::recursive_mutex> handlersLock (handlersMutex);
	if (handlers[event.id]) {
		handlers[event.id](event);
	}
}

void EventListener::HandleAllEvents() {
	OPTICK_EVENT();

	//This mutexes will locked in PollEvents
	std::lock_guard<std::recursive_mutex> eventsLock (eventsMutex);
	std::lock_guard<std::recursive_mutex> handlersLock (handlersMutex);

	if (Empty())
		return;

	while (!events.empty()) {
		Event event = events.front();
		events.pop();
		if (handlers[event.id]) {
			handlers[event.id](event);
		}
	}
}

bool EventListener::Empty() {
	std::lock_guard<std::recursive_mutex> eventsLock (eventsMutex);
	PollEvents();
	return events.empty();
}

void EventListener::RegisterHandler(size_t eventId, const EventListener::HandlerType &data) {
	std::lock_guard<std::recursive_mutex> handlersLock (handlersMutex);
	handlers[eventId] = data;
}

void EventListener::PollEvents() {
	OPTICK_EVENT();
	std::lock_guard<std::recursive_mutex> eventsLock (eventsMutex);
	std::lock_guard<std::recursive_mutex> handlersLock (handlersMutex);//To prevent inverse lock order

	std::lock_guard<std::recursive_mutex> rawLock (rawEventsMutex);
	if (rawEvents.empty())
		return;

	while (!rawEvents.empty()) {
		Event event = rawEvents.front();
		rawEvents.pop();
		if (handlers[event.id])
			events.push(event);
	}
}
