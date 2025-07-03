#ifndef EVENT_HPP
#define EVENT_HPP
#include <map>
#include <iostream>
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

    template<typename T> void publish(int eventType, T eventData) {
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
                    area->publish(eventType, eventData);
                }
                //std::cerr << "Error: No handler found for event type " << eventType << "." << std::endl;
            }
        }
    }

    void addArea(EventManager *area) {
        areas.push_back(area);
    }
};

class Axis3dEvent {
    public:
    glm::vec3 axis;
    float deltaTime;
    Axis3dEvent(glm::vec3 axis, float deltaTime) : axis(axis), deltaTime(deltaTime) {}
};

#endif