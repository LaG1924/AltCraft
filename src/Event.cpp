#include "Event.hpp"

std::list<EventListener*> EventSystem::listeners;
std::mutex EventSystem::listenersMutex;

EventListener::EventListener() {
	std::lock_guard<std::mutex> listenersLock(EventSystem::listenersMutex);
	EventSystem::listeners.push_back(this);
}

EventListener::~EventListener() {
	std::lock_guard<std::mutex> listenersLock(EventSystem::listenersMutex);
	EventSystem::listeners.remove(this);
}

void EventListener::HandleEvent() {
	std::lock_guard<std::mutex> lock(eventsQueueMutex);
	std::lock_guard<std::mutex> lockHandlers(handlersMutex);
	Event event = events.front();
	events.pop();	
	if (handlers[event.id]) {
		handlers[event.id](event);
	}
}

void EventListener::HandleAllEvents() {
	std::lock_guard<std::mutex> lock(eventsQueueMutex);
	std::lock_guard<std::mutex> lockHandlers(handlersMutex);
	while (!events.empty()) {
		Event event = events.front();
		events.pop();
		if (handlers[event.id]) {
			handlers[event.id](event);
		}
	}	
}

bool EventListener::NotEmpty() {
	std::lock_guard<std::mutex> lock(eventsQueueMutex);
	return !events.empty();
}

void EventListener::WaitEvent() {
	eventsQueueMutex.lock();
	while (events.empty()) {
		eventsQueueMutex.unlock();
		eventsQueueMutex.lock();
	}
	eventsQueueMutex.unlock();
}