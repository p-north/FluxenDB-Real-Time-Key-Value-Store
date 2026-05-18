#ifndef SERVER_H
#define SERVER_H

#include<atomic>
#include<string>
using namespace std;

class Server{
    private:
        int port;
        int server_socket;
        atomic<bool> running;

    public:
        Server(int port);
        void run();
        void shutdown();

};


#endif