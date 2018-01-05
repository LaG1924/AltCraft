#include "Event.hpp"

std::list<EventListener*> EventSystem::listeners;
std::recursive_mutex EventSystem::listenersMutex;

EventListener::EventListener() {
    EventSystem::listenersMutex.lock();
	EventSystem::listeners.push_back(this);
    EventSystem::listenersMutex.unlock();
}

EventListener::~EventListener() {
    EventSystem::listenersMutex.lock();
	EventSystem::listeners.remove(this);
    EventSystem::listenersMutex.unlock();
}

void EventListener::HandleEvent() {
	mutex.lock();
	Event event = events.front();
	events.pop();	
	if (handlers[event.id]) {
		handlers[event.id](event);
	}
	mutex.unlock();
}

void EventListener::HandleAllEvents() {
    if (!NotEmpty())
        return;

	mutex.lock();
	while (!events.empty()) {
		Event event = events.front();
		events.pop();
		if (handlers[event.id]) {
			handlers[event.id](event);
		}
	}
	mutex.unlock();
}

bool EventListener::NotEmpty() {
	bool ret = !events.empty();
	return ret;
}

void EventListener::WaitEvent() {
	mutex.lock();
	while (events.empty()) {
		mutex.unlock();
		mutex.lock();
	}
	mutex.unlock();
}