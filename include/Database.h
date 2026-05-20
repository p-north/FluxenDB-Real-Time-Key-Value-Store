#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <chrono>

class Database{
    public:
        // get singleton instance of class
        static Database& getInstance();

        // Common Commands
        bool flushAll();

        // TODO: Implement Key-value operations
        void set(const std::string& key, const std::string& value);
        bool get(const std::string& key, std::string&value);
        std::vector<std::string> keys();
        std::string type(const std::string& key);
        bool del(const std::string& key);
        // expire
        bool  expire(const std::string& key, const std::string& seconds);
        // rename
        bool rename(const std::string& oldKey, const std::string& newKey);

        // TODO: Implement List operations
        bool lset(const std::string &key, const int&index, const std::string&value);
        std::vector<std::string> lget(const std::string &key);
        size_t llen(const std::string &key);
        void lpush(const std::string &key, const std::vector<std::string>&values);
        void rpush(const std::string &key, const std::vector<std::string>&values);
        std::string lpop(const std::string &key);
        std::string rpop(const std::string &key);
        int lrem(const std::string &key, int count, const std::string&value);
        std::string lindex(const std::string &key, const int&index);

        

        
    

        // DB Persistance: Dump / load the database from a file
        bool dump(const std::string& filename);
        bool load(const std::string& filename);

    private:
        // constructors/destructors
        Database() = default;
        ~Database() = default;
        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;
        std::mutex db_mutex;

        // kv_store from string(key) -> string(value)
        std::unordered_map<std::string, std::string> kv_store;
        // list_store from string(key) -> list(value)
        std::unordered_map<std::string, std::vector<std::string>> list_store;
        // hash_store from string(key) -> map(value)
        std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hash_store;

        std::unordered_map<std::string, std::chrono::steady_clock::time_point> expirey_map;
};

#endif