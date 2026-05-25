#include <gtest/gtest.h>
#include "../include/Database.h"


//LIST TEST CASES
TEST(ListTests, LPUSHWorks) {
    Database& db = Database::getInstance();

    db.flushAll();
    db.lpush("mylist", {"a", "b", "c"});

    EXPECT_EQ(db.llen("mylist"),3);
}

TEST(DatabaseListEdgeCases, EmptyListOperations) {
    Database& db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.llen("missing"), 0);
    EXPECT_EQ(db.lpop("missing"), "");
    EXPECT_EQ(db.rpop("missing"), "");
}

TEST(DatabaseListEdgeCases, OutOfBoundIndex) {
    Database& db = Database::getInstance();
    db.flushAll();

    db.lpush("list", {"a", "b", "c"});

    EXPECT_EQ(db.lindex("list", 99999), "");
    EXPECT_FALSE(db.lset("list", 99999, "x"));
    EXPECT_EQ(db.lindex("list", -1), "");
    EXPECT_FALSE(db.lset("list", -1, "x"));
}

TEST(DatabaseListEdgeCases, PopUntilEmpty) {
    Database& db = Database::getInstance();
    db.flushAll();

    db.lpush("list", {"a", "b", "c"});

    EXPECT_NE(db.lpop("list"), "");
    EXPECT_NE(db.lpop("list"), "");
    EXPECT_NE(db.lpop("list"), "");
    EXPECT_EQ(db.lpop("list"), "");
}

TEST(DatabaseListEdgeCases, DuplicateValues) {
    Database& db = Database::getInstance();
    db.flushAll();

    db.lpush("list", {"a", "a", "a"});

    EXPECT_EQ(db.lrem("list", 0, "a"), 3);
}

TEST(DatabaseListEdgeCases, MixedValuesOrder) {
    Database& db = Database::getInstance();
    db.flushAll();

    db.rpush("list", {"1", "2"});
    db.lpush("list", {"0"});

    auto list = db.lget("list");

    EXPECT_EQ(list, std::vector<std::string>({"0", "1", "2"}));
}

TEST(DatabaseListEdgeCases, ComplexOperationSequence) {
    Database& db = Database::getInstance();
    db.flushAll();

    db.rpush("list", {"1", "2"});
    db.lpush("list", {"0"});
    db.lpop("list");   // removes "0"
    db.rpush("list", {"3"});

    auto list = db.lget("list");

    EXPECT_EQ(list, std::vector<std::string>({"1", "2", "3"}));
}

TEST(DatabaseListEdgeCases, LSetValueActuallyChangesList) {
    Database& db = Database::getInstance();
    db.flushAll();
    
    db.lpush("list", {"a", "b", "c"});//[c,b,a]
    
    db.lset("list", 1, "X");//[c,X,a]
    
    auto list = db.lget("list");
    
    EXPECT_EQ(list, std::vector<std::string>({"c", "X", "a"}));
}

TEST(DatabaseListEdgeCases, PopCorrectValues) {
    Database& db = Database::getInstance();
    db.flushAll();
    
    db.lpush("list", {"a", "b", "c"});//[c,b,a]
    
    EXPECT_EQ(db.lpop("list"), "c"); // [b,a]
    EXPECT_EQ(db.rpop("list"), "a"); // [b]
    EXPECT_EQ(db.lget("list"), std::vector<std::string>({"b"}));
    }

TEST(DatabaseListEdgeCases, LRemPartialForward) {
        Database& db = Database::getInstance();
        db.flushAll();
    
        db.lpush("list", {"a", "b", "a", "c", "a"});//[a,c,a,b,a]
    
        db.lrem("list", 2, "a");//[c,b,a]
    
        auto list = db.lget("list");
    
        EXPECT_EQ(list, std::vector<std::string>({"c", "b", "a"}));
    }

TEST(DatabaseListEdgeCases, LRemNegativeOrderCheck) {
        Database& db = Database::getInstance();
        db.flushAll();
    
        db.lpush("list", {"a", "b", "a", "c", "a"});//[a,c,a,b,a]
    
        db.lrem("list", -1, "a");//[a,c,a,b]
    
        auto list = db.lget("list");
    
        EXPECT_EQ(list, std::vector<std::string>({"a", "c", "a", "b"}));
    }

TEST(DatabaseListEdgeCases, EmptyStringValues) {
        Database& db = Database::getInstance();
        db.flushAll();
    
        db.lpush("list", {"", "a", ""});
    
        auto list = db.lget("list");
    
        EXPECT_EQ(list.size(), 3);
        EXPECT_EQ(list[0], "");
    }

TEST(DatabaseListEdgeCases, LargeListStressTest) {
        Database& db = Database::getInstance();
        db.flushAll();
    
        std::vector<std::string> big(10000, "x");
    
        db.lpush("list", big);
    
        EXPECT_EQ(db.llen("list"), 10000);
    }

// TEST(DatabaseListEdgeCases, KeyTypeMixing) {
//         Database& db = Database::getInstance();
//         db.flushAll();
    
//         db.set("k", "v");      
//         db.lpush("k", {"a", "b"});
    
//         EXPECT_EQ(db.type("k"), "list"); 
//     }