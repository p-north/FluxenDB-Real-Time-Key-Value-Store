#include <gtest/gtest.h>
#include "../include/Database.h"

TEST(HashOperations, HSetCreatesNewHashAndReturnsOneForNewField) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.hset("user", "name", "alice"), 1);
    EXPECT_TRUE(db.hexists("user", "name"));
}

TEST(HashOperations, HSetUpdatesExistingFieldAndReturnsZero) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.hset("user", "name", "alice"), 1);
    EXPECT_EQ(db.hset("user", "name", "bob"), 0);

    std::string value;
    EXPECT_EQ(db.hget("user", "name", value), "bob");
    EXPECT_EQ(value, "bob");
}

TEST(HashOperations, HGetReturnsStoredValueForExistingField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "age", "25");

    std::string value;
    EXPECT_EQ(db.hget("user", "age", value), "25");
    EXPECT_EQ(value, "25");
}

TEST(HashOperations, HGetReturnsEmptyStringForMissingField) {
    Database &db = Database::getInstance();
    db.flushAll();

    std::string value;
    EXPECT_EQ(db.hget("user", "missing", value), "");
    EXPECT_EQ(value, "");
}

TEST(HashOperations, HGetReturnsEmptyStringForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    std::string value;
    EXPECT_EQ(db.hget("missing", "field", value), "");
    EXPECT_EQ(value, "");
}

TEST(HashOperations, HExistsReturnsTrueForExistingField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    EXPECT_TRUE(db.hexists("user", "name"));
}

TEST(HashOperations, HExistsReturnsFalseForMissingField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    EXPECT_FALSE(db.hexists("user", "missing"));
}

TEST(HashOperations, HExistsReturnsFalseForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_FALSE(db.hexists("missing", "field"));
}

TEST(HashOperations, HDelReturnsTrueForExistingField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    EXPECT_TRUE(db.hdel("user", "name"));
    EXPECT_FALSE(db.hexists("user", "name"));
}

TEST(HashOperations, HDelReturnsFalseForMissingField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    EXPECT_FALSE(db.hdel("user", "missing"));
}

TEST(HashOperations, HDelReturnsFalseForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_FALSE(db.hdel("missing", "field"));
}

TEST(HashOperations, HDelRemovesOnlyTargetFieldAndPreservesOthers) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");

    EXPECT_TRUE(db.hdel("user", "name"));
    EXPECT_FALSE(db.hexists("user", "name"));
    EXPECT_TRUE(db.hexists("user", "age"));
}

TEST(HashOperations, MultipleFieldsCanBeStoredInTheSameHash) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.hset("user", "name", "alice"), 1);
    EXPECT_EQ(db.hset("user", "age", "25"), 1);
    EXPECT_EQ(db.hset("user", "city", "seattle"), 1);

    std::string name;
    std::string age;
    std::string city;

    EXPECT_EQ(db.hget("user", "name", name), "alice");
    EXPECT_EQ(db.hget("user", "age", age), "25");
    EXPECT_EQ(db.hget("user", "city", city), "seattle");
}

TEST(HashOperations, HSetCanStoreEmptyStringValues) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.hset("user", "nickname", ""), 1);

    std::string value;
    EXPECT_EQ(db.hget("user", "nickname", value), "");
    EXPECT_EQ(value, "");
}

TEST(HashOperations, HSetCanStoreEmptyFieldNames) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.hset("user", "", "value"), 1);

    std::string value;
    EXPECT_EQ(db.hget("user", "", value), "value");
    EXPECT_EQ(value, "value");
}

TEST(HashOperations, HSetWithRepeatedUpdatesPreservesLatestValue) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "name", "bob");
    db.hset("user", "name", "carol");

    std::string value;
    EXPECT_EQ(db.hget("user", "name", value), "carol");
    EXPECT_EQ(value, "carol");
}

TEST(HashOperations, HDelAfterRepeatedUpdatesRemovesCurrentValue) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "name", "bob");

    EXPECT_TRUE(db.hdel("user", "name"));
    EXPECT_FALSE(db.hexists("user", "name"));
}

TEST(HashOperations, FlushAllClearsAllHashData) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("account", "balance", "100");

    EXPECT_TRUE(db.flushAll());
    EXPECT_FALSE(db.hexists("user", "name"));
    EXPECT_FALSE(db.hexists("account", "balance"));
}

TEST(HashOperations, HashOperationsDoNotAffectOtherKeyTypes) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.set("plain", "value");
    db.hset("user", "name", "alice");

    std::string plainValue;
    EXPECT_TRUE(db.get("plain", plainValue));
    EXPECT_EQ(plainValue, "value");
    EXPECT_TRUE(db.hexists("user", "name"));
}

TEST(HashOperations, HGetAfterDeleteReturnsEmptyString) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hdel("user", "name");

    std::string value;
    EXPECT_EQ(db.hget("user", "name", value), "");
    EXPECT_EQ(value, "");
}

TEST(HashOperations, HSetHandlesManyFieldsInOneHash) {
    Database &db = Database::getInstance();
    db.flushAll();

    for (int i = 0; i < 100; ++i) {
        EXPECT_EQ(db.hset("user", "field" + std::to_string(i), "value" + std::to_string(i)), 1);
    }

    std::string value;
    EXPECT_EQ(db.hget("user", "field50", value), "value50");
    EXPECT_EQ(value, "value50");
}

TEST(HashOperations, HSetCanOverwriteFieldValueWithAnotherString) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "name", "bob");

    std::string value;
    EXPECT_EQ(db.hget("user", "name", value), "bob");
    EXPECT_EQ(value, "bob");
}

TEST(HashOperations, HDelOnLastFieldLeavesHashUnavailable) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    EXPECT_TRUE(db.hdel("user", "name"));
    EXPECT_FALSE(db.hexists("user", "name"));
    EXPECT_FALSE(db.hdel("user", "name"));
}
