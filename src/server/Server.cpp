#include "../../include/Server.h"
#include "../../include/CommandHandler.h"
#include "../../include/Database.h"
#include "../../include/Metrics.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>
#include <cstring>

#define MAX_EVENTS 256
#define MAX_MESSAGE_LEN 8192

static Server *globalServer = nullptr;

// Example: CTRL-C signal shutdown
void signalHandler(int signum)
{
    if (globalServer)
    {
        std::cout << "Caught signal " << signum << ", shutting down...\n";
        globalServer->shutdown();
    }
    exit(signum);
}

// Helper function to make server socket non-blocking
void makeNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        throw std::runtime_error("fcntl(F_GETFL) failed..\n");
    }

    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        throw std::runtime_error("fcntl(F_SETFL) failed..\n");
    }
}

void Server::setupSignalHandler()
{
    signal(SIGINT, signalHandler);
}

Server::Server(int port) : port(port), server_socket_fd(-1), running(true)
{
    globalServer = this;
    setupSignalHandler();
}

void Server::shutdown()
{
    running = false;
    if (server_socket_fd != -1)
        close(server_socket_fd);
    std::cout << "Server shutdown complete!\n";
}

void Server::run()
{

    server_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket_fd < 0)
    {
        std::cerr << "Error creating server socket\n";
        return;
    }

    // build socket
    int opt = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    memset((char *)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        std::cerr << "Error binding server socket\n";
        return;
    }

    if (listen(server_socket_fd, 10000) < 0)
    {
        std::cerr << "Error Listening ON Server Socket\n";
        return;
    }

    // make socket non blocking
    makeNonBlocking(server_socket_fd);

    std::cout << "Server listening on port " << port << "\n";

    commandHandler cmdHandler;

    struct epoll_event ev, events[MAX_EVENTS];
    int new_events, sock_conn_fd, epollfd;

    epollfd = epoll_create1(0);
    if (epollfd == -1)
    {
        std::cerr << "Error creating epoll..\n";
        return;
    }
    ev.events = EPOLLIN;
    ev.data.fd = server_socket_fd;

    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, server_socket_fd, &ev) == -1)
    {
        std::cerr << "Error adding new listening socket to epoll..\n";
        close(epollfd);
        return;
    }

    while (running)
    {

        new_events = epoll_wait(epollfd, events, MAX_EVENTS, 500); // 500ms timeout

        if (new_events == -1)
        {
            if (errno == EINTR)
                continue; // interrupted by signal, check running
            std::cerr << "Error in epoll_wait..\n";
            break;
        }

        // iterate thru each event
        for (int i = 0; i < new_events; i++)
        {

            // new connection?
            if (events[i].data.fd == server_socket_fd)
            {
                while (1)
                {
                    client_len = sizeof(client_addr);
                    sock_conn_fd = accept4(server_socket_fd, (struct sockaddr *)&client_addr, &client_len, SOCK_NONBLOCK);
                    if (sock_conn_fd == -1)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break; // no connection ready
                        std::cerr << "Error accepting new client connection: " << strerror(errno) << "\n";
                        break;
                    }

                    Metrics::getInstance().addClient();
                    std::cout << "\nCLIENT_SOCKET_FILE_DESCRIPTER " << sock_conn_fd << " CONNECTED\n";

                    ev.events = EPOLLIN | EPOLLRDHUP;
                    ev.data.fd = sock_conn_fd;
                    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock_conn_fd, &ev) == -1)
                    {
                        std::cerr << "Error adding new event to epoll..\n";
                        close(sock_conn_fd);
                        Metrics::getInstance().removeClient();
                    }
                }
            }
            else
            {
                char buffer[MAX_MESSAGE_LEN];
                memset(buffer, 0, sizeof(buffer));
                int newsockfd = events[i].data.fd;

                int bytes_received = recv(newsockfd, buffer, MAX_MESSAGE_LEN, 0);

                if (bytes_received == -1)
                {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        continue; // no data yet, not an error

                    std::cerr << "recv error on fd" << newsockfd << ": " << strerror(errno) << "\n";
                    close(newsockfd);
                    continue;
                }

                if (bytes_received > 0)
                {
                    // execute command and parse resp
                    std::string request(buffer, bytes_received);
                    std::string response = cmdHandler.processCommand(request);

                    // send response
                    ssize_t sent = send(newsockfd, response.c_str(), response.size(), 0);
                    if (sent == -1)
                    {
                        std::cerr << "send failed on fd " << newsockfd << ": " << strerror(errno) << "\n";
                    }
                }

                if (bytes_received == 0 || (events[i].events & EPOLLRDHUP))
                {
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, newsockfd, nullptr);
                    close(newsockfd);
                    std::cout << "\nCLIENT_SOCKET_FILE_DESCRIPTER " << newsockfd << " DISCONNECTED"
                                                                                    "\n";
                    Metrics::getInstance().removeClient();
                    continue;
                }
            }
        }
    }
    close(epollfd);

    // before shutdown check for persistent db
    if (Database::getInstance().dump("dump.my_db"))
        std::cout << "Database Dumped to dump.my_db\n";

    else
        std::cerr << "Error dumping database";
    // shutdown
}
