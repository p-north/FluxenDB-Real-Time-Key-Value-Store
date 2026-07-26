#include "../../include/Client.h"
#include <sys/socket.h>
#include <iostream>
#include <netinet/in.h>
#include <string>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
Client::Client(const std::string&host, int port):host_addr(host), port(port), sockfd(-1),is_connected(false){}
Client::~Client(){
    closeConnection();
}

bool Client::connectToServer(){
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1){
        std::cerr<<"Socket creation failed\n";
        return false;
    }
    sockaddr_in serverAddr{};
    serverAddr.sin_family=AF_INET;
    serverAddr.sin_port = htons(port);

    //converting host IPaddress from presentation(human readable) to network byte format

    if(inet_pton(AF_INET, host_addr.c_str(), &serverAddr.sin_addr)<=0){
        std::cerr<<"Invalid host IP address\n";
        return false;
    }
    if(connect(sockfd, (sockaddr*)&serverAddr, sizeof(serverAddr))<0){
        std::cerr << "Connection failed\n";
        return false;
    }
    is_connected=true;
    return true;
}
std::string Client::parseSendCommand(std::vector<std::string>&tokens){
   std::string command;
    std::string CRLF = "\r\n";
    command+= "*"+std::to_string(tokens.size())+CRLF;
    for(std::string& token: tokens){
        command+="$"+std::to_string(token.size())+CRLF;
        command+=token+CRLF;
    }
    //cout<<"COMMAND IS: "+command+"\n";
    return command;
}
std::string Client::parseResponse(std::string& response){
    std::string CRLF="\r\n";
    if(response.empty()){
        return "NO RESPONSE: CHECK SERVER SINCE SERVER SENDS RESPONSE FOR EVERY REQUEST\n";
    }
    char type = response[0];
    // Simple String
    if(type=='+'){
        return response.substr(1, response.find(CRLF)-1);
    }
    // Error
    else if (type == '-') {
        return "ERROR: " + response.substr(1, response.find("\r\n") - 1);
    }

    // Integer
    else if (type == ':') {
        return response.substr(1, response.find("\r\n") - 1);
    }

    // Bulk String ($)
    else if (type == '$') {
        size_t pos = response.find("\r\n");
        int len = stoi(response.substr(1, pos - 1));

        if (len == -1) return "NULL";

        return response.substr(pos + 2, len);
    }
    //array
    else if(type=='*'){
        //*1\r\n$6\r\nbottle\r\n
        size_t i = 0;

        // --- Parse array header ---
        size_t end = response.find("\r\n", i);
        int numElements = std::stoi(response.substr(i + 1, end - i - 1));
        i = end + 2;
    
        std::string result;
    
        for (int k = 0; k < numElements; k++) {
    
            // --- Parse bulk string header ($len) ---
            end = response.find("\r\n", i);
            int len = std::stoi(response.substr(i + 1, end - i - 1));
            i = end + 2;
    
            if (len == -1){
                result+="NULL ";
                continue;
            }
            // --- Extract string ---
            result += response.substr(i, len)+" ";
            i += len + 2; // skip \r\n
        }
    
        return result;
    }
    else{
        //THIS SHOULD NOT BE EXECUTED
        return response;
    }
}
//$
bool Client::sendCommand(const std::string&cmd){
    std::string data = cmd+"\r\n";
    ssize_t bytes_sent= send(sockfd, data.c_str(), data.size(),0);
    return bytes_sent>0;
}

std::string Client::receiveResponse(){
    char buffer[4096];
    memset(buffer, 0, sizeof(buffer));
    ssize_t bytes_received = recv(sockfd, buffer, sizeof(buffer)-1, 0);
     if (bytes_received <= 0) {
        return "there is an error reading the response from the server";
    }
    return std::string(buffer, bytes_received);
}

void Client::closeConnection(){
    if(sockfd!=-1){
        close(sockfd);
        sockfd=-1;
    }
    is_connected=false;
}
