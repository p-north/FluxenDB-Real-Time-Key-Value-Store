#ifndef SERVER_H
#define SERVER_H

#include<atomic>
#include<string>


class Server{
    private:
        int port;
        int server_socket_fd;
        std::atomic<bool> running;

        // Seup signal handling for graceful shutdwn (ctrl + c)
        void setupSignalHandler();

    public:
        Server(int port);
        void run();
        void shutdown();

};


#endif