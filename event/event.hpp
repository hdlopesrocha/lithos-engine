#ifndef EVENT_HPP
#define EVENT_HPP
#include <map>
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#define EVENT_PAINT_BRUSH 1
#define EVENT_CLOSE_WINDOW 2
#define EVENT_VECTOR_3D_0 3
#define EVENT_VECTOR_3D_1 4
#define EVENT_PREVIOUS_PAGE 5
#define EVENT_NEXT_PAGE 6

class Event {
    public:
    int type;
    Event(int type) : type(type) {}
    int getType() const {
        return type;
    }
};

class Axis3dEvent : public Event {
    public:
    glm::vec3 axis;
    float deltaTime;
    Axis3dEvent(int type, glm::vec3 axis, float deltaTime) : Event(type), axis(axis), deltaTime(deltaTime) {}
};

class BinaryEvent : public Event {
    public:
    bool value;
    BinaryEvent(int type, bool value) : Event(type), value(value) {}
};


class EventHandlerBase {
    public:
    virtual ~EventHandlerBase() = default;
    // Base class for event handlers, can be extended for specific event types
};

template<typename T> class EventHandler : public EventHandlerBase {
    public:
    virtual ~EventHandler() = default;
    virtual void handle(T eventData) = 0;
};


class EventManager {
    std::map<int, EventHandlerBase*> handlers;
    std::vector<EventManager*> areas; 
    bool enabled = true;
    public:

    void subscribe(int eventType, EventHandlerBase *handler) {
        handlers[eventType] = handler;
    }

    template<typename T> void publish(T eventData) {
        static_assert(std::is_base_of<Event, T>::value, "T must derive from Event");
        int eventType = eventData.getType();
        if(enabled) {
            auto it = handlers.find(eventType);
            if (it != handlers.end()) {
                EventHandler<T> * handler = dynamic_cast<EventHandler<T>*>(it->second);
                if (handler) {
                    handler->handle(eventData);
                } else {
                    std::cerr << "Error: Handler for event type " << eventType << " does not match data type." << std::endl;
                }
            } else {
                for(const auto& area : areas) {
                    area->publish(eventData);
                }
                //std::cerr << "Error: No handler found for event type " << eventType << "." << std::endl;
            }
        }
    }

    void setEnabled(bool enabled) {
        this->enabled = enabled;
    }

    void addArea(EventManager *area) {
        areas.push_back(area);
    }
};

class EventManagerGroup {
    std::vector<EventManager*> eventManagers;
    size_t selectedManager = 0;
    public:
    void addEventManager(EventManager *eventManager) {
        eventManagers.push_back(eventManager);
        select(0);
    }

    void select(int index) {
        selectedManager = index % eventManagers.size();
        for(size_t i = 0; i < eventManagers.size(); ++i) {
            if(i == selectedManager) {
                eventManagers[i]->setEnabled(true);
            } else {
                eventManagers[i]->setEnabled(false);
            }
        }

    }

    void next() {
        select(++selectedManager);
    }

    void previous() {
        select(--selectedManager);
    }
};

#endif