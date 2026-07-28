#include "../../include/CommandHandler.h"
#include "../../include/Database.h"
#include "../../include/Metrics.h"
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <chrono>
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

commandHandler::commandHandler() {}

std::string commandHandler::processCommand(const std::string &commandLine)
{
    // use RESP parser
    auto tokens = parseRespCommand(commandLine);
    if (tokens.empty())
        return "-Error: Empty command\r\n";

    // // DEBUGGIN ONLY
    // for(auto&t : tokens)  std::cout << t << "\n";

    // get the first command
    std::string cmd = tokens[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);
    std::ostringstream response;
    // connnect to database
    Database &db = Database::getInstance();

    auto latency_start = std::chrono::steady_clock::now();
    

    // check commands
    // common commands--------------------
    if (cmd == "PING")
    {
        response << "+PONG\r\n";
        Metrics::getInstance().incrementSuccessfullCommand(cmd);
    }
    else if (cmd == "ECHO")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: ECHO requires a messag\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {   
            response << "+" << tokens[1] << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "FLUSHALL")
    {
        db.flushAll();
        response << "+OK\r\n";
    }
    // Key-value operations------------
    else if (cmd == "SET")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: SET requires key and value\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            db.set(tokens[1], tokens[2]);
            response << "+OK\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "GET")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: GET requires key\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::string value;
            if (db.get(tokens[1], value))
                response << "$" << value.size() << "\r\n"
                         << value << "\r\n";
            else
                response << "$-1\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "KEYS")
    {
        std::vector<std::string> allKeys = db.keys();
        response << "*" << allKeys.size() << "\r\n";
        for (const auto &key : allKeys)
        {
            response << "$" << key.size() << "\r\n"
                     << key << "\r\n";
        }
        Metrics::getInstance().incrementSuccessfullCommand(cmd);
    }
    else if (cmd == "TYPE")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: TYPE requires key\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            response << "+" << db.type(tokens[1]) << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "DEL" || cmd == "UNLINK")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: " << cmd << "requires key" << "\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            bool res = db.del(tokens[1]);
            response << ":" << (res ? 1 : 0) << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "EXPIRE")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: EXPIRE requires key and time in seconds\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            if (db.expire(tokens[1], tokens[2]))
                response << "+OK\r\n";
            else
                response << "$-1\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "RENAME")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: RENAME requires old key name and new key name\r\n";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            db.rename(tokens[1], tokens[2]);
            response << "+OK\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    // List operations
    else if (cmd == "LSET")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: LSET requires key, position index and a value to set";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            if (db.lset(tokens[1], std::stoi(tokens[2]), tokens[3]))
            {
                response << "+OK\r\n";
                Metrics::getInstance().incrementSuccessfullCommand(cmd);
            }
            else
            {
                response << "-Error: Index out of range\r\n";
                Metrics::getInstance().incrementFailureCommand(cmd);
            }
        }
    }
    else if (cmd == "LGET")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: LGET requires a list name (key)";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::vector<std::string> list = db.lget(tokens[1]);
            response << "*" << list.size() << "\r\n";
            for (const auto &item : list)
            {
                response << "$" << item.size() << "\r\n"
                         << item << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "LLEN")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: LLEN requires a list name (key)";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            size_t length = db.llen(tokens[1]);
            response << ":" << length << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "LPUSH")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: LPUSH requires key, and a value";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            int size = db.lpush(tokens[1], std::vector<std::string>(tokens.begin() + 2, tokens.end()));
            response << ":" << std::to_string(size) << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "RPUSH")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: RPUSH requires key, and a value";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            int size = db.rpush(tokens[1], std::vector<std::string>(tokens.begin() + 2, tokens.end()));
            response << ":" << std::to_string(size) << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "LPOP")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: LPOP requires the list name (key)";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::string value = db.lpop(tokens[1]);
            if (value.empty())
            {
                response << "$-1\r\n";
                
            }
            else
            {
                response << "$" << value.size() << "\r\n"
                         << value << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "RPOP")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: RPOP requires the list name (key)";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::string value = db.rpop(tokens[1]);
            if (value.empty())
            {
                response << "$-1\r\n";
            }
            else
            {
                response << "$" << value.size() << "\r\n"
                         << value << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "LREM")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: LREM requires key, count and a value to be removed.";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            int count = db.lrem(tokens[1], std::stoi(tokens[2]), tokens[3]);
            response << ":" << count << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "LINDEX")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: LINDEX requires key and an elemnent position";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::string value = db.lindex(tokens[1], std::stoi(tokens[2]));
            if (value.empty())
            {
                response << "$-Error: Index out of range\r\n";
                Metrics::getInstance().incrementFailureCommand(cmd);
            }
            else
            {
                response << "$" << value.size() << "\r\n"
                         << value << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }

    // Hash operations
    else if (cmd == "HSET")
    {
        if (tokens.size() < 4)
        {
            response << "-Error: HSET requires key, field and value";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            int result = db.hset(tokens[1], tokens[2], tokens[3]);
            response << ":" << result << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HGET")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: HGET requires key and field";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::string value;
            std::string result = db.hget(tokens[1], tokens[2], value);
            if (result.empty())
            {
                response << "$-1\r\n"; // null bulk string
            }
            else
            {
                response << "$" << value.size() << "\r\n"
                         << value << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HEXISTS")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: HEXISTS requires key and field";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            bool exists = db.hexists(tokens[1], tokens[2]);
            response << ":" << (exists ? 1 : 0) << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HDEL")
    {
        if (tokens.size() < 3)
        {
            response << "-Error: HDEL requires key and field";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            bool deleted = db.hdel(tokens[1], tokens[2]);
            response << ":" << (deleted ? 1 : 0) << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HLEN")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: HLEN requires key";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            size_t length = db.hlen(tokens[1]);
            response << ":" << length << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HKEYS")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: HKEYS requires a key";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::vector<std::string> keys = db.hkeys(tokens[1]);
            response << "*" << keys.size() << "\r\n";
            for (const auto key : keys)
            {
                response << "$" << key.size() << "\r\n"
                         << key << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HVALS")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: HVALS requires a key";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::vector<std::string> values = db.hvals(tokens[1]);
            response << "*" << values.size() << "\r\n";
            for (const auto val : values)
            {
                response << "$" << val.size() << "\r\n"
                         << val << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HGETALL")
    {
        if (tokens.size() < 2)
        {
            response << "-Error: HGETALL requires a key";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            std::unordered_map<std::string, std::string> allMap = db.hgetall(tokens[1]);
            response << "*" << allMap.size() << "\r\n";
            for (const auto [field, value] : allMap)
            {
                response << "$" << field.size() << "\r\n"
                         << field << "\r\n";
                response << "$" << value.size() << "\r\n"
                         << value << "\r\n";
            }
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else if (cmd == "HMSET")
    {
        if (tokens.size() < 4)
        {
            response << "-Error: HMSET requires a minimum of key, field and value";
            Metrics::getInstance().incrementFailureCommand(cmd);
        }
        else
        {
            for (int i = 2; i < tokens.size(); i += 2)
            {
                db.hset(tokens[1], tokens[i], tokens[i + 1]);
            }
            response << "+OK" << "\r\n";
            Metrics::getInstance().incrementSuccessfullCommand(cmd);
        }
    }
    else
    {
        response << "-Error: Unknown command '" << cmd << "'\r\n";
        Metrics::getInstance().incrementFailureCommand(cmd);
    }

    auto latency_end = std::chrono::steady_clock::now();
    double duration_seconds =
        std::chrono::duration<double>(latency_end - latency_start).count();
    Metrics::getInstance().recordCommandLatency(cmd, duration_seconds);

    return response.str();
}
