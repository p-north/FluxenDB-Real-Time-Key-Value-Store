#include "../../include/Metrics.h"
#include <fstream>
#include <sstream>

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

size_t Metrics::getMemoryUsageKB(){
    std::ifstream file("/proc/self/status");
    std::string line;
    while(std::getline(file, line)){
        if(line.rfind("VmRSS:", 0)==0){
            std::istringstream iss(line);
            std::string key;
            size_t value;
            iss >> key >> value;
            return value;
        }
    }
    return 0;
}

std::string Metrics::exportMetrics()
{
    std::string output;

    // expected prometheus format
    /*
        # HELP <metric_name> <description>
        # TYPE <metric_name> <type>
        <metric_name> <value>
    */

    output += "# HELP fluxendb_commands_failure_total Total failed commands\n";
    output += "# TYPE fluxendb_commands_failure_total counter\n";
    output += "fluxendb_commands_failure_total ";
    output += std::to_string(totalFailureComands.load());
    output += "\n";
    
    output += "# HELP fluxendb_commands_success_total Total successful commands\n";
    output += "# TYPE fluxendb_commands_success_total counter\n";
    output += "fluxendb_commands_success_total ";
    output += std::to_string(totalSuccessCommands.load());
    output += "\n";


    output += "# HELP fluxendb_commands_total Total number of commands processed\n";
    output += "# TYPE fluxendb_commands_total counter\n";
    output += "fluxendb_commands_total ";
    output += std::to_string(totalFailureComands.load()+totalSuccessCommands.load());
    output += "\n";
    

    output += "# HELP fluxendb_connected_clients Current number of connected clients\n";
    output += "# TYPE fluxendb_connected_clients gauge\n";
    output += "fluxendb_connected_clients ";
    output += std::to_string(clients.load());
    output += "\n";

    output += "# HELP fluxendb_memory_usage Process memory usage in KB\n";
    output += "# TYPE fluxendb_memory_usage gauge\n";
    output += "fluxendb_memory_usage ";
    output += std::to_string(getMemoryUsageKB());
    output += "\n\n";

    return output;
}
