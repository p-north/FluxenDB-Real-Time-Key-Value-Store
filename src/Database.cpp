#include "../include/Database.h"

#include <mutex>
#include <fstream>

// Singleton pattern -> returns db object instance
Database &Database::getInstance()
{
    static Database instance;
    return instance;
}

/*
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

}