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

#define EVENT_PAINT_BRUSH 1
#define EVENT_CLOSE_WINDOW 2
#define EVENT_VECTOR_3D_0 3
#define EVENT_VECTOR_3D_1 4
#define EVENT_PREVIOUS_PAGE 5
#define EVENT_NEXT_PAGE 6
#define EVENT_COMPONENT_3D_0 7
#define EVENT_COMPONENT_3D_1 8
#define EVENT_COMPONENT_3D_2 9
#define EVENT_FLOAT_0_X 10
#define EVENT_FLOAT_0_Y 11
#define EVENT_FLOAT_0_Z 12
#define EVENT_FLOAT_1_X 13
#define EVENT_FLOAT_1_Y 14
#define EVENT_FLOAT_1_Z 15
#define EVENT_FLOAT_2_X 16
#define EVENT_FLOAT_2_Y 17
#define EVENT_FLOAT_2_Z 18


class Event {
    public:
    int type;
    Event(int type) : type(type) {}
    int getType() const {
        return type;
    }
};

class FloatEvent : public Event {
    public:
    float value;
    FloatEvent(int type, float value) : Event(type), value(value) {}
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


#endif