#include "handler.hpp"

ScaleHandler::ScaleHandler(float * value) : value(value) {

}

void ScaleHandler::handle(FloatEvent * event) {
    float time = event->duration;
    *value += time * event->value*event->deltaTime;
}

