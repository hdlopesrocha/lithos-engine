#include "command.hpp"


CloseWindowCommand::CloseWindowCommand(LithosApplication &app) : app(app) {

}

void CloseWindowCommand::execute(float value) {
    app.close();
}
