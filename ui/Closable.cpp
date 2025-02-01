#include "ui.hpp"

void Closable::show() {
    open = true;
}

void Closable::hide(){
    open = false;
}

bool Closable::isOpen(){
    return open;
}
