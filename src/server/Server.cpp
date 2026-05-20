#include "../../include/Server.h"
#include "../../include/CommandHandler.h"
#include "../../include/Database.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <vector>
#include <thread>
#include <cstring>

static Server* globalServer = nullptr;

// Example: CTRL-C signal shutdown
void signalHandler(int signum){
    if(globalServer)
        std::cout << "Caught signal " << signum << ", shutting down...\n";
        globalServer->shutdown();
    exit(signum);
}

void Server::setupSignalHandler(){
    signal(SIGINT, signalHandler);
}

Server::Server(int port) : port(port), server_socket(-1), running(true) {
    globalServer = this;
    setupSignalHandler();
}

void Server::shutdown(){
    running = false;
    if(server_socket != -1) close(server_socket);
    std::cout << "Server shutdown complete!\n";
}

void Server::run(){
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(server_socket < 0) {
        std::cerr << "Error creating server socket\n";
        return;
    }

    // build socket
    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if(bind(server_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0){
        std::cerr << "Error binding server socket\n";
        return;
    }

    if(listen(server_socket, 10) < 0){
        std::cerr << "Error Listening ON Server Socket\n";
        return;
    }

    std::cout << "Server listening on port " << port << "\n";

    std::vector<std::thread> threads;
    commandHandler cmdHandler;

    while(running){
        int client_socket = accept(server_socket, nullptr, nullptr);
        if(client_socket < 0)
            if(running)
                std::cerr << "Error accepting client connections\n";
           break;
        
        threads.emplace_back([client_socket, &cmdHandler](){
            char buffer[1024];
            while(true){
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
                if(bytes <= 0) break;
                std::string request(buffer, bytes);
                std::string response = cmdHandler.processCommand(request);
                send(client_socket, response.c_str(), response.size(), 0);
            }
            close(client_socket);
        });
    }

    for(auto&t : threads){
        // if thread is joinable then join t
        if (t.joinable()) t.join();
    }

    // before shutdown check for persistent db
    if(Database::getInstance().dump("dump.my_db"))
         std::cout  << "Database Dumped to dump.my_db\n";

    else
        std::cerr << "Error dumping database";
    // shutdown

    
}
