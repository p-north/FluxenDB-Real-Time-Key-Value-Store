#include "../../include/Server.h"
#include "../../include/Database.h"
#include "../utils/MetricsServer.cpp"
#include <iostream>
#include <thread>
#include <chrono>



int main(int argc, char* argv[]){

    std::thread metrics(startMetricsServer);
    metrics.detach();
    std::cout << "Metrics http server listening on port " << METRICS_PORT << std::endl;

    int port = 6379;    // default port
    if(argc >=2) port = std::stoi(argv[1]);      // client provided port

    if(Database::getInstance().load("dump.my_db"))
        std::cout << "Database loaded from dump file\n";
    else
        std::cout << "No dump found or load failed; starting with an empty database.\n";


    Server server(port);   

    // run the server
    server.run();
    
    return 0;
}