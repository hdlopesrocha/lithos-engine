#include "handler.hpp"

ScaleHandler::ScaleHandler(float * value) : value(value) {

}

void ScaleHandler::handle(FloatEvent * event) {
    *value += event->value*event->deltaTime;
}

