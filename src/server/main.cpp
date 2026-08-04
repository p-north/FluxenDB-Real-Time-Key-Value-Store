#include "../../include/Server.h"
#include "../../include/Database.h"
#include "../utils/MetricsServer.cpp"
#include <iostream>
#include <thread>
#include <chrono>



int main(int argc, char* argv[]){

    std::thread metrics(startMetricsServer);
    std::cout << "Metrics http server listening on port " << METRICS_PORT << std::endl;
    
    if(Database::getInstance().load("dump.my_db"))
    std::cout << "Database loaded from dump file\n";
    else
    std::cout << "No dump found or load failed; starting with an empty database.\n";
    
    Server server = argc < 2 ? Server(6379): Server(std::stoi(argv[1])); 


    // Background persistance: dump the database every 300s
    std::thread persistanceThread([](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(300));
            // dump/load the datbase
            if(!Database::getInstance().dump("dump.my_db"))
                std::cerr << "Error dumping database\n";
            else    
                std::cout  << "SERVER_MAIN: Database Dumped to dump.my_db\n";

        }
    });
    persistanceThread.detach();

    // run the server
    server.run();

    // join metrics server thread
    metrics.join();
    
    return 0;
}