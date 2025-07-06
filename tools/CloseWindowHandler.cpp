#include "tools.hpp"


CloseWindowHandler::CloseWindowHandler(LithosApplication &app) : app(app) {

}

void CloseWindowHandler::handle(Event value) {
    app.close();
}
