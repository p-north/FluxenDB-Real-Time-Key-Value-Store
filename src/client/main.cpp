#include "../../include/Client.h"
#include <thread>
#include <vector>
#include <iostream>
#include <algorithm>
#include <sstream>

std::string toUpper(std::string s){
    std::transform(s.begin(),s.end(), s.begin(), [](unsigned char c){
        return toupper(c); 
    });
    return s;
}
int main(int argc, char* argv[]){
    std::cout<<"Welcome to FluxenDB CLI: \n";
    //connect to the server
    Client client("127.0.0.1", 6379);
    if(!client.connectToServer()){
        return -1;
    }
   //accept user commands
    while(true){
        std::cout<<"Enter a command: ";
        std::string command;
        getline(std::cin, command);
        std::vector<std::string>tokens;
        std::stringstream ss(command);
        std::string token;
        while(ss>> token){
            tokens.push_back(token);
        }
        tokens[0]=toUpper(tokens[0]);

        if(tokens[0]=="EXIT"){
            break;
        }

        if(tokens.size()<2){
            std::cout<<"Please enter a key and a value\n";
        }
        command=client.parseSendCommand(tokens);
        client.sendCommand(command);
        std::string response = client.receiveResponse();
        response = client.parseResponse(response);
        std::cout<<response<<"\n";
    }  
    std::cout<<"See You Again\n";
}