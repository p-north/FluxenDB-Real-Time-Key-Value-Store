#include "../include/Database.h"

#include <mutex>
#include <fstream>
#include <sstream>

// Singleton pattern -> returns db object instance
Database &Database::getInstance()
{
    static Database instance;
    return instance;
}

/*
    Database persistance

    Memory -> File - dump()
    File -> Memory - load()

    K = key value
    L = List
    H = Hash

*/

bool Database::dump(const std::string &filename)
{
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ofstream ofs(filename, std::ios::binary);
    if (!ofs)
        return false;

    // save in dump file
    
    // Key-value 
    for (const auto &kv : kv_store)
    {
        ofs << "K " << kv.first << " " << kv.second << "\n";
    }
    // List 
    for (const auto &kv : list_store)
    {
        ofs << "L " << kv.first;
        for (const auto &item : kv.second)
        {
            ofs << " " << item;
        }
        ofs << "\n";
    }
    // Hash 
    for (const auto& kv : hash_store){
        ofs << "H " << kv.first;
        for(const auto& it : kv.second){
            ofs << " " << it.first << ":" << it.second;
        }
        ofs << "\n";
    }

        return true;
}

bool Database::load(const std::string &filename){
    std::lock_guard<std::mutex> lock(db_mutex);
    std::ifstream ifs(filename, std::ios::binary);
    if(!ifs) return false;

    // clear all data structures
    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;
    // reading from db file
    while(std::getline(ifs, line)){
        std::istringstream iss(line);
        char type;
        iss >> type;
        if(type == 'K'){
            std::string key, value;
            iss >> key >> value;
            // add to data structure
            kv_store[key] = value;
        }
        else if(type == 'L'){
            // get the key
            std::string key;
            iss >> key;
            // get the value
            std::string item;
            std::vector<std::string> list;
            while(iss >> item){
                list.push_back(item);
            }
            list_store[key] = list;
        }
        else if(type == 'H'){
            std::string key;
            iss >> key;
            std::unordered_map<std::string, std::string> hash;
            std::string pair;
            while(iss >> pair){
                // split from colon
                auto pos = pair.find(':');
                if(pos != std::string::npos){
                    std::string field = pair.substr(0, pos);
                    std::string value = pair.substr(pos+1);
                    hash[field] = value;
                }

            }
            hash_store[key] = hash;

        }

    }

}