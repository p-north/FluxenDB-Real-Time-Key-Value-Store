#include <gtest/gtest.h>
#include "../include/Database.h"

TEST(ListTests, LPUSHWorks) {
    Database& db = Database::getInstance();

    db.flushAll();
    db.lpush("mylist", {"a", "b", "c"});

    EXPECT_EQ(db.llen("mylist"),3);
}