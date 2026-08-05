#include "../../include/Server.h"
#include "../../include/CommandHandler.h"
#include "../../include/Database.h"
#include "../../include/Metrics.h"
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <vector>
#include <thread>
#include <cstring>
#include <errno.h> 
#include <sys/epoll.h>
#include <fcntl.h>
#include <chrono>

const int MAX_EVENTS = 1000;
auto lastDump = std::chrono::steady_clock::now();
const auto dumpInterval = std::chrono::seconds(300);

bool setNonBlocking(int fd){
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) == 0;
}

static Server* globalServer = nullptr;
// Example: CTRL-C signal shutdown
void signalHandler(int signum){
    std::cout << "Caught signal " << signum << ", shutting down...\n";
    globalServer->shutdown();
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

    if(listen(server_socket, SOMAXCONN) < 0){
        std::cerr << "Error Listening ON Server Socket\n";
        return;
    }

    std::cout << "Server listening on port " << port << "\n";

    setNonBlocking(server_socket);
    int epollFd = epoll_create1(0);
    if(epollFd<0){
        std::cerr<<"epoll_create1 failed: "<< strerror(errno);
        return;
    }

    struct epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = server_socket;
    int res = epoll_ctl(epollFd, EPOLL_CTL_ADD, server_socket, &ev);
    if(res!=0){
        std::cerr << "epoll_ctl(server) failed: " << strerror(errno) << "\n";
        return;
    }
    struct epoll_event events[MAX_EVENTS];
    commandHandler cmdHandler;
    char buffer[4096];
    while(running){
         int n= epoll_wait(epollFd, events, MAX_EVENTS, 300);
        if(n<0){
            if(errno == EINTR) continue; 
            std::cerr<<"epoll_wait failed: "<<strerror(errno) << "\n";
            break;
        }
        for(int i=0;i<n;i++){
            int fd = events[i].data.fd;
            if(fd==server_socket){
                while(true){
                    int client_socket = accept(server_socket, nullptr, nullptr);
                    if(client_socket < 0){
                        if(errno == EAGAIN) break;
                        std::cerr << "accept failed: " << strerror(errno) << "\n";
                        break;
                    }
                    setNonBlocking(client_socket);
                    struct epoll_event cev{};
                    cev.events  = EPOLLIN;
                    cev.data.fd = client_socket;
                    int res = epoll_ctl(epollFd, EPOLL_CTL_ADD, client_socket, &cev);
                    if(res!=0){
                        std::cerr<<"epoll control failed"<<strerror(errno)<<"\n";
                        close(client_socket);
                        continue;
                    }
                    Metrics::getInstance().addClient();
                }
            }else{         
                memset(buffer, 0, sizeof(buffer));
                int bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
                if(bytes > 0){
                std::string request(buffer, bytes);
                std::string response = cmdHandler.processCommand(request);
                send(fd, response.c_str(), response.size(), 0);
                }else if(bytes == 0 || (errno != EAGAIN && errno != EWOULDBLOCK)){
                    // Peer closed, or a real error. Deregister before closing.
                    epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, nullptr);
                    close(fd);
                    Metrics::getInstance().removeClient();
                }
            }
        }
        // Periodic persistence.
        auto now = std::chrono::steady_clock::now();
        if(now - lastDump >= dumpInterval){
            if(Database::getInstance().dump("dump.my_db"))
                std::cout << "Database dumped to dump.my_db\n";
            else
                std::cerr << "Error dumping database\n";
            lastDump = now;
        }

    }
    close(epollFd);

    // before shutdown check for persistent db
    if(Database::getInstance().dump("dump.my_db"))
         std::cout  << "Database Dumped to dump.my_db\n";

    else
        std::cerr << "Error dumping database";
    // shutdown 
}













    // std::vector<std::thread> threads;
    // commandHandler cmdHandler;

    // while(running){
    //     int client_socket = accept(server_socket, nullptr, nullptr);
    //     std::cout<<"\nCLIENT_SOCKET_FILE_DESCRIPTER "<<client_socket<< " CONNECTED""\n";
        
        
    //     if(client_socket < 0){
    //         std::cerr << "Error accepting client connections\n";
    //         continue;

    //     }
        
    //     Metrics::getInstance().addClient();
    //     threads.emplace_back([client_socket, &cmdHandler](){
    //         char buffer[4096];
            
    //         while(true){
    //             memset(buffer, 0, sizeof(buffer));
    //             int bytes = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    //             if(bytes <= 0) break;
    //             std::string request(buffer, bytes);
    //             std::string response = cmdHandler.processCommand(request);
    //             send(client_socket, response.c_str(), response.size(), 0);
    //         }
    //         close(client_socket);
    //             std::cout<<"\nCLIENT_SOCKET_FILE_DESCRIPTER "<<client_socket<< " DISCONNECTED""\n";
    //         Metrics::getInstance().removeClient();
    //     });
    // }

    // for(auto&t : threads){
    //     // if thread is joinable then join t
    //     if (t.joinable()) t.join();
    // }