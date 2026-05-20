#include "../../include/CommandHandler.h"
#include "../../include/Database.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>

// RESP parser:
// *2\r\n\$4\r\n\PING\r\n\$4\r\n\TEST\r\n
// *2 -> array has 2 elements
// $4 -> next string has 4 characters
// PING
// TEST

std::vector<std::string> parseRespCommand(const std::string &input)
{
    std::vector<std::string> tokens;
    if (input.empty())
        return tokens;

    // if does not start with '*', fallback to splitting by whitespaces
    if (input[0] != '*')
    {
        std::istringstream iss(input);
        std::string token;
        while (iss >> token)
            tokens.push_back(token);
        return tokens;
    }

    // else parse the RESP
    size_t pos = 0;
    // Expect '*' followed by number of elements
    if (input[pos] != '*')
        return tokens;
    pos++; // skip '*'

    // crlf = carriage return (\r), line feed (\n)
    size_t crlf = input.find("\r\n", pos);
    if (crlf == std::string::npos)
        return tokens;

    int numElements = std::stoi(input.substr(pos, crlf - pos));
    pos = crlf + 2;

    for (int i = 0; i < numElements; ++i)
    {
        if (pos >= input.size() || input[pos] != '$')
            break; // format error
        pos++;     // skip '$'

        crlf = input.find("\r\n", pos);
        if (crlf == std::string::npos)
            break;
        int len = std::stoi(input.substr(pos, crlf - pos));
        pos = crlf + 2;

        if (pos + len > input.size())
            break;
        std::string token = input.substr(pos, len);
        tokens.push_back(token);
        pos += len + 2; // sip token and CRLF

    }
    return tokens;
}

commandHandler::commandHandler(){}

std::string commandHandler::processCommand(const std::string& commandLine){
    // use RESP parser
    auto tokens = parseRespCommand(commandLine);
    if(tokens.empty()) return "-Error: Empty command\r\n";

    // // DEBUGGIN ONLY
    // for(auto&t : tokens)  std::cout << t << "\n";
      
    

    // get the first command
    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    std::ostringstream response;
    // connnect to database
    Database& db = Database::getInstance();


    // check commands 
    // common commands--------------------
    if(cmd == "PING"){
        response << "+PONG\r\n";
    }
    else if(cmd == "ECHO"){
        if(tokens.size() < 2){
            response << "-Error: ECHO requires a messag\r\n";
        }
        else{
            response << "+" << tokens[1] << "\r\n";

        }

    }
    else if(cmd == "FLUSHALL"){
        db.flushAll();
        response << "+OK\r\n";
    }
    // TODO: Key-value operations------------
    else if (cmd == "SET"){
        if(tokens.size() < 3){
            response << "-Error: SET requires key and value\r\n";
        }
        else{
            db.set(tokens[1], tokens[2]);
            response << "+OK\r\n";
        }
    }
    else if (cmd == "GET"){
         if(tokens.size() < 2){
            response << "-Error: GET requires key\r\n";
        }
        else{
            std::string value;
            if(db.get(tokens[1], value))
                response << "$" << value.size() << "\r\n" << value << "\r\n";
            else
                response << "$-1\r\n";
        }
    }
    else if (cmd == "KEYS"){
        std::vector<std::string> allKeys = db.keys();
        response << "*" << allKeys.size() << "\r\n";
        for(const auto& key : allKeys){
            response << "$" << key.size() << "\r\n" << key << "\r\n";
        }
    }
    else if (cmd == "TYPE"){
        if(tokens.size() < 2){
            response << "-Error: TYPE requires key\r\n";
        }
        else{
            response << "+" << db.type(tokens[1]) << "\r\n";
        }
    }
    else if (cmd == "DEL" || cmd == "UNLINK"){
        if(tokens.size() < 2){
            response << "-Error: " << cmd << "requires key" << 
        "\r\n";
        }
        else{
            bool res = db.del(tokens[1]);
            response << ":" << (res ? 1:0) << "\r\n";
        }
    }
    else if(cmd == "EXPIRE"){
        if(tokens.size() < 3){
            response << "-Error: EXPIRE requires key and time in seconds\r\n";
        }
        else{
            db.expire(tokens[1], tokens[2]);
            response << "+DK\r\n";

        }
    }
    else if (cmd == "RENAME"){
        if(tokens.size() < 3){
            response << "-Error: RENAME requires old key name and new key name\r\n";
        }
        else{
            db.rename(tokens[1], tokens[2]);
            response << "+DK\r\n";

        }
    }
    // TODO: List operations
    // TODO: Hash operations
    else{
        response << "-Error: unknown command\r\n";
    }

    return response.str();
}
