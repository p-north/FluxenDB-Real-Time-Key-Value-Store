#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <vector>

class Client{
public:
    Client(const std::string& host, int port);
    ~Client();
    bool connectToServer();
    std::string parseSendCommand(std::vector<std::string>&tokens);
    std::string parseResponse(std::string&response);
    bool sendCommand(const std::string& cmd);
    std::string receiveResponse();
    void closeConnection();
private:
    std::string host_addr;
    int port;
    int sockfd;
    bool is_connected;
};
#endif