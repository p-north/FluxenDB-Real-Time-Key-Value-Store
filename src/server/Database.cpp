#include "../../include/Database.h"
#include <fstream>
#include <mutex>
#include <sstream>
#include <chrono>
#include <algorithm>

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

// Implement Key-value operations
bool Database::flushAll()
{

    kv_store.clear();
    list_store.clear();
    hash_store.clear();
    return true;
}
void Database::set(const std::string &key, const std::string &value)
{

    kv_store[key] = value;
}
bool Database::get(const std::string &key, std::string &value)
{

    auto it = kv_store.find(key);
    if (it != kv_store.end())
    {
        value = it->second;
        return true;
    }
    return false;
}
std::vector<std::string> Database::keys()
{

    std::vector<std::string> keys;
    for (const auto it : kv_store)
    {
        keys.push_back(it.first);
    }
    for (const auto it : list_store)
    {
        keys.push_back(it.first);
    }
    for (const auto it : kv_store)
    {
        keys.push_back(it.first);
    }
    return keys;
}
std::string Database::type(const std::string &key)
{

    if (kv_store.find(key) != kv_store.end())
    {
        return "string";
    }
    if (list_store.find(key) != list_store.end())
    {
        return "list";
    }
    if (hash_store.find(key) != hash_store.end())
    {
        return "hash";
    }
    else
        return "none";
}
bool Database::del(const std::string &key)
{

    bool erased = false;
    erased |= kv_store.erase(key) > 0;
    erased |= list_store.erase(key) > 0;
    erased |= hash_store.erase(key) > 0;
    return erased;
}
// expire
bool Database::expire(const std::string &key, const std::string &seconds)
{

    bool exist = (kv_store.find(key) != kv_store.end()) ||
                 (list_store.find(key) != list_store.end()) ||
                 (hash_store.find(key) != hash_store.end());
    if (!exist)
        return false;
    expirey_map[key] = std::chrono::steady_clock::now() + std::chrono::seconds(std::stoll(seconds));
    return true;
}
// rename
bool Database::rename(const std::string &oldKey, const std::string &newKey)
{

    bool found = false;

    auto itKv = kv_store.find(oldKey);
    if (itKv != kv_store.end())
    {
        kv_store[newKey] = itKv->second;
        kv_store.erase(itKv);
        found = true;
    }

    auto itList = list_store.find(oldKey);
    if (itList != list_store.end())
    {
        list_store[newKey] = itList->second;
        list_store.erase(itList);
        found = true;
    }

    auto itHash = hash_store.find(oldKey);
    if (itHash != hash_store.end())
    {
        hash_store[newKey] = itHash->second;
        hash_store.erase(itHash);
        found = true;
    }

    auto itExpire = expirey_map.find(oldKey);
    if (itExpire != expirey_map.end())
    {
        expirey_map[newKey] = itExpire->second;
        expirey_map.erase(itExpire);
        found = true;
    }

    return found;
}

//-----------LIST OPERATIONS---------------------

bool Database::lset(const std::string &key, const int &index, const std::string &value)
{

    std::vector<std::string> &list = list_store[key];
    if (index < 0 || index >= list.size())
    {
        return false;
    }
    list[index] = value;
    return true;
}

std::vector<std::string> Database::lget(const std::string &key)
{

    return list_store[key];
}

size_t Database::llen(const std::string &key)
{

    return list_store[key].size();
}

int Database::lpush(const std::string &key, const std::vector<std::string> &values)
{

    std::vector<std::string> &list = list_store[key];
    for (const std::string &value : values)
    {
        list.insert(list.begin(), value);
    }

    return list.size();
}

int Database::rpush(const std::string &key, const std::vector<std::string> &values)
{

    std::vector<std::string> &list = list_store[key];
    for (const std::string &value : values)
    {
        list.insert(list.end(), value);
    }
    return list.size();
}

std::string Database::lpop(const std::string &key)
{

    std::vector<std::string> &list = list_store[key];

    if (!list.empty())
    {
        std::string value = list.front();
        list.erase(list.begin());
        return value;
    }
    return "";
}

std::string Database::rpop(const std::string &key)
{

    std::vector<std::string> &list = list_store[key];
    if (!list.empty())
    {
        std::string value = list.back();
        list.pop_back();
        return value;
    }
    return "";
}

int Database::lrem(const std::string &key, int count, const std::string &value)
{

    if (list_store.find(key) == list_store.end())
    {
        return 0;
    }
    std::vector<std::string> &list = list_store[key];
    int removed = 0;
    if (count == 0)
    {
        int actualSize = list.size();
        // list.erase(std::remove(list.begin(), list.end(), value), list.end());
        removed = actualSize - list.size();
    }
    else if (count > 0)
    {
        for (auto it = list.begin(); it != list.end() && count > 0;)
        {
            if (*it == value)
            {
                it = list.erase(it);
                count--;
                removed++;
            }
            else
            {
                it++;
            }
        }
    }
    else if (count < 0)
    {
        count = -count;
        for (int i = list.size() - 1; i >= 0 && count > 0; i--)
        {
            if (list[i] == value)
            {
                list.erase(list.begin() + i);
                count--;
                removed++;
            }
        }
    }
    return removed;
}

std::string Database::lindex(const std::string &key, const int &index)
{

    std::vector<std::string> &list = list_store[key];
    if (index < 0 || index >= list.size())
    {
        return "";
    }
    return list[index];
}

//-----------Hash OPERATIONS---------------------
bool Database::hset(const std::string &key, const std::string &field, const std::string &value)
{


    std::unordered_map<std::string, std::string> &hash = hash_store[key];
    if (hash.find(field) == hash.end())
    {
        hash[field] = value;
        return 1; // new field added
    }
    hash[field] = value;
    return 0; // field updated
}

std::string Database::hget(const std::string &key, const std::string &field, std::string &value)
{


    if (hash_store.find(key) == hash_store.end())
    {
        return "";
    }
    std::unordered_map<std::string, std::string> &hash = hash_store[key];
    if (hash.find(field) == hash.end())
    {
        return "";
    }
    value = hash[field];
    return value;
}

bool Database::hexists(const std::string &key, const std::string &field)
{


    if (hash_store.find(key) == hash_store.end())
    {
        return false;
    }
    std::unordered_map<std::string, std::string> &hash = hash_store[key];
    return hash.find(field) != hash.end();
}

bool Database::hdel(const std::string &key, const std::string &field)
{

    if (hash_store.find(key) == hash_store.end())
    {
        return false;
    }
    std::unordered_map<std::string, std::string> &hash = hash_store[key];
    return hash.erase(field) > 0;
}

size_t Database::hlen(const std::string &key)
{

    return hash_store[key].size();
}

std::vector<std::string> Database::hkeys(const std::string &key)
{

    std::vector<std::string> result;
    for (const auto [field, value] : hash_store[key])
    {
        result.push_back(field);
    }
    return result;
}

std::vector<std::string> Database::hvals(const std::string &key)
{

    std::vector<std::string> result;
    for (const auto [field, value] : hash_store[key])
    {
        result.push_back(value);
    }
    return result;
}

std::unordered_map<std::string, std::string> Database::hgetall(const std::string &key)
{

    std::unordered_map<std::string, std::string> allMap;
    for (const auto [field, value] : hash_store[key])
    {
        allMap[field] = value;
    }
    return allMap;
}

// -----------------------Database load/dump implmentation
// dumps database -> iterates through all data structures, adds to datbase file
bool Database::dump(const std::string &filename)
{

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
    for (const auto &kv : hash_store)
    {
        ofs << "H " << kv.first;
        for (const auto &it : kv.second)
        {
            ofs << " " << it.first << ":" << it.second;
        }
        ofs << "\n";
    }

    return true;
}

bool Database::load(const std::string &filename)
{

    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs)
        return false;

    // clear all data structures
    kv_store.clear();
    list_store.clear();
    hash_store.clear();

    std::string line;
    // reading from db file
    while (std::getline(ifs, line))
    {
        // line->K key value
        std::istringstream iss(line);
        char type;
        iss >> type;
        if (type == 'K')
        {
            std::string key, value;
            iss >> key >> value;
            // add to data structure
            kv_store[key] = value;
        }
        else if (type == 'L')
        {
            // line->L key value1 value2 value3
            //  get the key
            std::string key;
            iss >> key;
            // get the value
            std::string item;
            std::vector<std::string> list;
            while (iss >> item)
            {
                list.push_back(item);
            }
            list_store[key] = list;
        }
        else if (type == 'H')
        {
            // line->H key field1:value1 field2:value2 field3:value3
            std::string key;
            iss >> key;
            std::unordered_map<std::string, std::string> hash;
            std::string pair;
            while (iss >> pair)
            {
                // split from colon
                auto pos = pair.find(':');
                if (pos != std::string::npos)
                {
                    std::string field = pair.substr(0, pos);
                    std::string value = pair.substr(pos + 1);
                    hash[field] = value;
                }
            }
            hash_store[key] = hash;
        }
    }
    return true;
}
