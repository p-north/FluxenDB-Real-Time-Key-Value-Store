#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <string>


class commandHandler {
    public:
        commandHandler();
        // Process the command from client and return RESP-formatted response
        std::string processCommand(const std::string& commandLine);

        
};
#endif