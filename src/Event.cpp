#include "Event.hpp"
#include <easylogging++.h>

std::queue<Event> EventAgregator::eventsToHandle;
std::mutex EventAgregator::queueMutex;
bool EventAgregator::isStarted = false;
std::vector<EventListener *> EventAgregator::listeners;
std::mutex EventAgregator::listenersMutex;

EventListener::EventListener() {
	EventAgregator::RegisterListener(*this);
}

EventListener::~EventListener() {
	EventAgregator::UnregisterListener(*this);
}

void EventListener::PushEvent(Event event) {
	eventsMutex.lock();
	handlersMutex.lock();
	if (handlers[event.type]) {
		events.push(event);
	}
	handlersMutex.unlock();
	eventsMutex.unlock();
}

void EventListener::DirectCall(Event event)
{
    handlersMutex.lock();
    if (handlers[event.type])
        handlers[event.type](event.data);
    handlersMutex.unlock();
}

bool EventListener::IsEventsQueueIsNotEmpty() {
	eventsMutex.lock();
	bool value = !events.empty();
	eventsMutex.unlock();
	return value;
}


void EventListener::RegisterHandler(EventType type, EventListener::HandlerFunc handler) {
	handlersMutex.lock();
	handlers[type] = handler;
	handlersMutex.unlock();
}

void EventListener::HandleEvent() {
	eventsMutex.lock();
	if (events.empty()) {
		eventsMutex.unlock();
		return;
	}
	Event event = events.front();
	events.pop();
	eventsMutex.unlock();
	handlersMutex.lock();
	auto function = handlers[event.type];
	handlersMutex.unlock();
	function(event.data);
}


void EventAgregator::RegisterListener(EventListener &listener) {
	listenersMutex.lock();
	//LOG(WARNING) << "Registered handler " << &listener;
	listeners.push_back(&listener);
	listenersMutex.unlock();
}

void EventAgregator::UnregisterListener(EventListener &listener) {
	listenersMutex.lock();
	//LOG(WARNING) << "Unregistered handler " << &listener;
	listeners.erase(std::find(listeners.begin(), listeners.end(), &listener));
	listenersMutex.unlock();
}

void EventAgregator::PushEvent(EventType type, EventData data) {
	Event event;
	event.type = type;
	event.data = data;
    queueMutex.lock();
	eventsToHandle.push(event);
    queueMutex.unlock();
	if (!isStarted) {
		isStarted = true;
		std::thread(&EventAgregator::EventHandlingLoop).detach();
	}
}

void EventAgregator::DirectEventCall(EventType type, EventData data)
{
    Event event {type, data};
    listenersMutex.lock();
    for (auto &listener : listeners) {
        listenersMutex.unlock();
        listener->DirectCall(event);
        listenersMutex.lock();
    }
    listenersMutex.unlock();
}

void EventAgregator::EventHandlingLoop() {
    LoopExecutionTimeController timer(std::chrono::milliseconds(5));
	while (true) {
		queueMutex.lock();
		if (!eventsToHandle.empty()) {
			auto queue = std::move(eventsToHandle);            
			queueMutex.unlock();

			while (!queue.empty()) {
				auto event = queue.front();
				listenersMutex.lock();
				for (auto &listener : listeners) {
					listener->PushEvent(event);
				}
				listenersMutex.unlock();
				queue.pop();
			}

			queueMutex.lock();
		}
		queueMutex.unlock();
        timer.Update();
	}
}