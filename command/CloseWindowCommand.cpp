#include "command.hpp"


CloseWindowCommand::CloseWindowCommand(LithosApplication &app) : app(app) {

}

void CloseWindowCommand::execute(const float &value) {
    app.close();
}
