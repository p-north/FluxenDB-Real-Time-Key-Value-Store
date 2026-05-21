#include "../../include/Client.h"
#include <thread>
#include <vector>
#include <iostream>

using namespace std;

string toUpper(string s){
    transform(s.begin(),s.end(), s.begin(), [](unsigned char c){
        return toupper(c); 
    });
    return s;
}
int main(int argc, char* argv[]){
    cout<<"Welcome to Redis CLI: \n";
    //connect to the server
    Client client("127.0.0.1", 6379);
    if(!client.connectToServer()){
        return -1;
    }
   //accept user commands
    while(true){
        cout<<"Enter a command: ";
        string command;
        getline(cin, command);
        vector<string>tokens;
        stringstream ss(command);
        string token;
        while(ss>> token){
            tokens.push_back(token);
        }
        tokens[0]=toUpper(tokens[0]);

        if(tokens[0]=="EXIT"){
            break;
        }

        if(tokens.size()<2){
            cout<<"Please enter a key and a value\n";
        }
        command=client.parseSendCommand(tokens);
        client.sendCommand(command);
        string response = client.receiveResponse();
        response = client.parseResponse(response);
        cout<<response<<"\n";
    }  
    cout<<"See You Again\n";
    
}