#include "command.hpp"


CloseWindowCommand::CloseWindowCommand(LithosApplication &app) : app(app) {

}

void CloseWindowCommand::execute(Event value) {
    app.close();
}
