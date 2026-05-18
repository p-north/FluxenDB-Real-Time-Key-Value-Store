#include "../include/server.h"
#include "../include/database.h"
#include <iostream>
#include <thread>
#include <chrono>



int main(int argc, char* argv[]){

    int port = 6379;    // default port
    if(argc >=2) port = std::stoi(argv[1]);      // client provided port


    Server server(port);   
    
    // Background persistance: dump the database every 300s
    std::thread persistanceThread([](){
        while(true){
            std::this_thread::sleep_for(std::chrono::seconds(300));
            // dump/load the datbase
            if(!Database::getInstance().dump("dump.my_db"))
                std::cerr << "Error dumping database\n";
            else    
                std::cout  << "Database Dumped to dump.my_db\n";

        }
    });
    persistanceThread.detach();

    // run the server
    server.run();
    
    return 0;
}