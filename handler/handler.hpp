#ifndef HANDLER_HPP
#define HANDLER_HPP
#include <map>
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "../event/event.hpp"

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

class TranslateHandler : public EventHandler<Axis3dEvent>{
    Camera &camera;
    glm::vec3 &vector;
    public:
    TranslateHandler(Camera &camera, glm::vec3 &vector);
    void handle(Axis3dEvent value) override ;
};

class ScaleHandler : public EventHandler<Axis3dEvent>{
    glm::vec3 &vector;
    public:
    ScaleHandler(glm::vec3 &vector);
    void handle(Axis3dEvent value) override ;
};

class RotateHandler : public EventHandler<Axis3dEvent>{
    Camera &camera;
    glm::quat &quaternion;
    public:
    RotateHandler(Camera &camera, glm::quat &quaternion);
    void handle(Axis3dEvent value) override ;
};


class ControlEventManagerGroupHandler : public EventHandler<BinaryEvent> {
    EventManagerGroup &eventManagerGroup;
    public:
    ControlEventManagerGroupHandler(EventManagerGroup &eventManagerGroup) : eventManagerGroup(eventManagerGroup) {}
    void handle(BinaryEvent event) override {
        if(event.value) {
            eventManagerGroup.next();
        } else {
            eventManagerGroup.previous();
        }
    }
};

#endif