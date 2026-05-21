#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <vector>
using namespace std;
class Client{
public:
    Client(const string& host, int port);
    ~Client();
    bool connectToServer();
    string parseSendCommand(vector<string>&tokens);
    string parseResponse(string&response);
    bool sendCommand(const string& cmd);
    string receiveResponse();
    void closeConnection();
private:
    string host_addr;
    int port;
    int sockfd;
    bool is_connected;
};
#endif