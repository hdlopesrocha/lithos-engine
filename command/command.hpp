#ifndef COMMAND_HPP
#define COMMAND_HPP

#include "../gl/gl.hpp"
#include "../tools/tools.hpp"

template<typename T> class ICommand {
    public:
    virtual ~ICommand() = default;
    virtual void execute(T value) = 0;
};



#endif