#include "../../include/Client.h"
#include <thread>
#include <vector>
#include <iostream>

int main(){
    Client client("127.0.0.1", 6379);
    if(!client.connectToServer()){
        return -1;
    }
    client.sendCommand("*1\r\n$4\r\nPING\r\n");
    cout<<client.receiveResponse()<<"\n";
}