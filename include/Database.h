#ifndef DATABASE_H
#define DATABSE_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>

class Database{
    public:
        // get singleton instance of class
        static Database& getInstance();

        // Persistance: Dump / oad the database from a file
        bool dump(const std::string& filename);
        bool load(const std::string& filename);

    private:
        // constructors/destructors
        Database() = default;
        ~Database() = default;
        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;

        // kv_store from string(key) -> string(value)
        std::unordered_map<std::string, std::string> kv_store;
        // list_store from string(key) -> list(value)
        std::unordered_map<std::string, std::vector<std::string>> list_store;
        // hash_store from string(key) -> map(value)
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;

        std::mutex db_mutex;
};

#endif