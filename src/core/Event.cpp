#include <core/Event.hpp>
#include <easylogging++.h>

std::queue <Event> EventAgregator::eventsToHandle;
std::mutex EventAgregator::queueMutex;
bool EventAgregator::isStarted = false;
std::vector<EventListener*> EventAgregator::listeners;
std::mutex EventAgregator::listenersMutex;

void EventAgregator::EventHandlingLoop() {
    while (true) {
        queueMutex.lock();
        if (!eventsToHandle.empty()) {
            auto queue = eventsToHandle;
            while (!eventsToHandle.empty())
                eventsToHandle.pop();
            queueMutex.unlock();

            while (!queue.empty()) {
                auto event = queue.front();
                listenersMutex.lock();
                for (auto& listener : listeners) {
                    LOG(INFO)<<"Listener notified about event";
                    listener->PushEvent(event);
                }
                listenersMutex.unlock();
                queue.pop();
            }

            queueMutex.lock();
        }
        queueMutex.unlock();
    }
}

void EventAgregator::RegisterListener(EventListener &listener) {
    listenersMutex.lock();
    LOG(INFO)<<"Registered handler "<<&listener;
    listeners.push_back(&listener);
    listenersMutex.unlock();
}

void EventAgregator::UnregisterListener(EventListener &listener) {
    listenersMutex.lock();
    LOG(INFO)<<"Unregistered handler "<<&listener;
    listeners.erase(std::find(listeners.begin(), listeners.end(), &listener));
    listenersMutex.unlock();
}



EventListener::EventListener() {
    EventAgregator::RegisterListener(*this);
}

EventListener::~EventListener() {
    EventAgregator::UnregisterListener(*this);
}

void EventListener::PushEvent(Event event) {
    eventsMutex.lock();
    LOG(INFO)<<"Pushed event to queue";
    events.push(event);
    eventsMutex.unlock();
}

/*void EventListener::RegisterHandler(EventType type, std::function<void(void*)> handler) {
    handlers[type] = handler;
}*/

bool EventListener::IsEventsQueueIsNotEmpty() {
    eventsMutex.lock();
    bool value = !events.empty();
    eventsMutex.unlock();
    return value;
}