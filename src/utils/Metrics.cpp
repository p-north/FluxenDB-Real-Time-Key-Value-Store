#include "../../include/Metrics.h"

// Singleton pattern object
Metrics &Metrics::getInstance()
{
    static Metrics instance;
    return instance;
}

void Metrics::incrementSuccessfullCommand(const std::string& command){
    totalSuccessCommands++;
}

void Metrics::incrementFailureCommand(const std::string& command){
    totalFailureComands++;
}

void Metrics::addClient()
{
    clients++;
}

void Metrics::removeClient()
{
    clients--;
}

std::string Metrics::exportMetrics()
{
    std::string output;

   
    output += "total_commands_failure ";
    output += std::to_string(totalFailureComands.load());
    output += "\n";
    
    output += "total_commands_successfull ";
    output += std::to_string(totalSuccessCommands.load());
    output += "\n";


    output += "total_commands ";
    output += std::to_string(totalSuccessCommands.load()+totalFailureComands.load());
    output += "\n";

    output += "total_connected_clients ";
    output += std::to_string(clients.load());
    output += "\n";

    return output;
}
