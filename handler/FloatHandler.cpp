#include "handler.hpp"



FloatHandler::FloatHandler(float * floatPtr) : floatPtr(floatPtr) {

}

void FloatHandler::handle(FloatEvent * value) {
    *floatPtr += value->value*value->deltaTime;
} 
