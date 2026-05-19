#include "../../include/client.h"
#include <sys/socket.h>
#include <iostream>

#include <string>
Clinet::Client(const string&host, int port):host_addr(host), port(port), sockfd(-1),connected(false){}
Client::~Client(){
    closeConnection();
}

bool Client::connectToServer(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0)
    if(sockfd==-1){
        cerr<<"Socket creation failed\n"
        return false;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port = htons(port)

    //converting host IPaddress from presentation(human readable) to network byte format

    if(inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr)<=0){
        cerr<<"Invalid host IP address\n";
        return false;
    }
    if(connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr))<0){
        cerr << "Connection failed\n";
        return false;
    }
    is_connected=true;
    return true;
}

bool client::sendCommand(const string&cmd){
    string data = cmd+"\r\n";
    ssize_t bytes_sent= send(sockfd, data.c_str(), data.size(),0);
    return bytes_sent>0;
}

string Client::receiveResponse(){
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = rec(sockfd, buffer, sizeof(buffer)-1, 0 )
    return string(buffer, bytes_received);
}

void Client::closeConnection(){
    if(sockfd!=-1){
        close(sockfd);
        sockfd=-1;
    }
    is_connected=false;
}
