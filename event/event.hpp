#ifndef EVENT_HPP
#define EVENT_HPP
#include <map>
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include "../math/math.hpp"

enum EventType {
    EVENT_PAINT_BRUSH = 1,
    EVENT_CLOSE_WINDOW,
    EVENT_VECTOR_3D_0,
    EVENT_VECTOR_3D_1,
    EVENT_PREVIOUS_PAGE,
    EVENT_NEXT_PAGE,
    EVENT_COMPONENT_3D_0,
    EVENT_COMPONENT_3D_1,
    EVENT_COMPONENT_3D_2,
    EVENT_FLOAT_0_X,
    EVENT_FLOAT_0_Y,
    EVENT_FLOAT_0_Z,
    EVENT_FLOAT_1_X,
    EVENT_FLOAT_1_Y,
    EVENT_FLOAT_1_Z,
    EVENT_FLOAT_2_X,
    EVENT_FLOAT_2_Y,
    EVENT_FLOAT_2_Z
};


class Event {
    public:
    EventType type;
    Event(EventType type) : type(type) {}
    EventType getType() const {
        return type;
    }
};

class FloatEvent : public Event {
    public:
    float value;
    FloatEvent(EventType type, float value) : Event(type), value(value) {}
};

class Axis3dEvent : public Event {
    public:
    glm::vec3 axis;
    float deltaTime;
    Axis3dEvent(EventType type, glm::vec3 axis, float deltaTime) : Event(type), axis(axis), deltaTime(deltaTime) {}
};

class BinaryEvent : public Event {
    public:
    bool value;
    BinaryEvent(EventType type, bool value) : Event(type), value(value) {}
};


#endif