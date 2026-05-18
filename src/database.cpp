#include "../include/database.h"

// Singleton pattern -> returns db object instance
Database& Database::getInstance(){
    static Database instance;
    return instance;
}

bool Database::dump(const std::string& filename){

}

bool Database::load(const std::string& filename){

}