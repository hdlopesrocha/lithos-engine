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


#endif