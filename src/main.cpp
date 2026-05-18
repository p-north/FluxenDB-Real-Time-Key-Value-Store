#include "../include/server.h"
#include <iostream>

using namespace std;

int main(int argc, char* argv[]){

    int port = 6379;    // default port
    if(argc >=2) port = stoi(argv[1]);      // user provided port


    Server server(port);


    
    return 0;
}