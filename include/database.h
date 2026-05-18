#ifndef DATABASE_H
#define DATABSE_H

#include <string>

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
};

#endif