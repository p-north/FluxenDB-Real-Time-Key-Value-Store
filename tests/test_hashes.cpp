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

// -----------------------------------------------------------------------
// HLEN
// -----------------------------------------------------------------------

TEST(HashOperations, HLenReturnsZeroForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_EQ(db.hlen("missing"), 0);
}

TEST(HashOperations, HLenReturnsOneAfterSingleHSet) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    EXPECT_EQ(db.hlen("user"), 1);
}

TEST(HashOperations, HLenReturnsCorrectCountForMultipleFields) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hset("user", "city", "seattle");

    EXPECT_EQ(db.hlen("user"), 3);
}

TEST(HashOperations, HLenDoesNotIncrementOnFieldUpdate) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "name", "bob");

    EXPECT_EQ(db.hlen("user"), 1);
}

TEST(HashOperations, HLenDecrementsAfterHDel) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hdel("user", "name");

    EXPECT_EQ(db.hlen("user"), 1);
}

TEST(HashOperations, HLenReturnsZeroAfterAllFieldsDeleted) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hdel("user", "name");

    EXPECT_EQ(db.hlen("user"), 0);
}

// -----------------------------------------------------------------------
// HKEYS
// -----------------------------------------------------------------------

TEST(HashOperations, HKeysReturnsEmptyVectorForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_TRUE(db.hkeys("missing").empty());
}

TEST(HashOperations, HKeysReturnsSingleFieldName) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    auto keys = db.hkeys("user");
    EXPECT_EQ(keys.size(), 1);
    EXPECT_EQ(keys[0], "name");
}

TEST(HashOperations, HKeysReturnsAllFieldNames) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hset("user", "city", "seattle");

    auto keys = db.hkeys("user");
    EXPECT_EQ(keys.size(), 3);

    std::vector<std::string> sorted_keys = keys;
    std::sort(sorted_keys.begin(), sorted_keys.end());
    EXPECT_EQ(sorted_keys, std::vector<std::string>({"age", "city", "name"}));
}

TEST(HashOperations, HKeysDoesNotContainDeletedField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hdel("user", "name");

    auto keys = db.hkeys("user");
    EXPECT_EQ(keys.size(), 1);
    EXPECT_EQ(keys[0], "age");
}

TEST(HashOperations, HKeysCountMatchesHLen) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("session", "token", "abc");
    db.hset("session", "expires", "3600");
    db.hset("session", "user_id", "42");

    EXPECT_EQ(db.hkeys("session").size(), db.hlen("session"));
}

// -----------------------------------------------------------------------
// HVALS
// -----------------------------------------------------------------------

TEST(HashOperations, HValsReturnsEmptyVectorForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_TRUE(db.hvals("missing").empty());
}

TEST(HashOperations, HValsReturnsSingleValue) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    auto vals = db.hvals("user");
    EXPECT_EQ(vals.size(), 1);
    EXPECT_EQ(vals[0], "alice");
}

TEST(HashOperations, HValsReturnsAllValues) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hset("user", "city", "seattle");

    auto vals = db.hvals("user");
    EXPECT_EQ(vals.size(), 3);

    std::vector<std::string> sorted_vals = vals;
    std::sort(sorted_vals.begin(), sorted_vals.end());
    EXPECT_EQ(sorted_vals, std::vector<std::string>({"25", "alice", "seattle"}));
}

TEST(HashOperations, HValsReflectsUpdatedValue) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "name", "bob");

    auto vals = db.hvals("user");
    EXPECT_EQ(vals.size(), 1);
    EXPECT_EQ(vals[0], "bob");
}

TEST(HashOperations, HValsDoesNotContainValueOfDeletedField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hdel("user", "name");

    auto vals = db.hvals("user");
    EXPECT_EQ(vals.size(), 1);
    EXPECT_EQ(vals[0], "25");
}

TEST(HashOperations, HValsCountMatchesHLen) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("session", "token", "abc");
    db.hset("session", "expires", "3600");

    EXPECT_EQ(db.hvals("session").size(), db.hlen("session"));
}

// -----------------------------------------------------------------------
// HGETALL
// -----------------------------------------------------------------------

TEST(HashOperations, HGetAllReturnsEmptyMapForMissingKey) {
    Database &db = Database::getInstance();
    db.flushAll();

    EXPECT_TRUE(db.hgetall("missing").empty());
}

TEST(HashOperations, HGetAllReturnsSingleFieldValuePair) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    auto result = db.hgetall("user");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["name"], "alice");
}

TEST(HashOperations, HGetAllReturnsAllFieldValuePairs) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hset("user", "city", "seattle");

    auto result = db.hgetall("user");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result["name"], "alice");
    EXPECT_EQ(result["age"], "25");
    EXPECT_EQ(result["city"], "seattle");
}

TEST(HashOperations, HGetAllReflectsUpdatedFieldValue) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "name", "bob");

    auto result = db.hgetall("user");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result["name"], "bob");
}

TEST(HashOperations, HGetAllExcludesDeletedField) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hdel("user", "name");

    auto result = db.hgetall("user");
    EXPECT_EQ(result.size(), 1);
    EXPECT_EQ(result.count("name"), 0);
    EXPECT_EQ(result["age"], "25");
}

TEST(HashOperations, HGetAllSizeMatchesHLen) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("session", "token", "abc");
    db.hset("session", "expires", "3600");
    db.hset("session", "user_id", "42");

    EXPECT_EQ(db.hgetall("session").size(), db.hlen("session"));
}

TEST(HashOperations, HGetAllKeysMatchHKeys) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");

    auto all = db.hgetall("user");
    auto keys = db.hkeys("user");

    for (const auto &k : keys) {
        EXPECT_EQ(all.count(k), 1);
    }
}

TEST(HashOperations, HGetAllValuesMatchHVals) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");

    auto all = db.hgetall("user");
    auto vals = db.hvals("user");

    std::vector<std::string> all_vals;
    for (const auto &[field, value] : all) {
        all_vals.push_back(value);
    }

    std::sort(all_vals.begin(), all_vals.end());
    std::sort(vals.begin(), vals.end());
    EXPECT_EQ(all_vals, vals);
}

// -----------------------------------------------------------------------
// HMSET (multi-field set — implemented as repeated hset calls)
// -----------------------------------------------------------------------

TEST(HashOperations, HMSetStoresMultipleFieldsAtOnce) {
    Database &db = Database::getInstance();
    db.flushAll();

    // HMSET user name alice age 25 city seattle
    db.hset("user", "name", "alice");
    db.hset("user", "age", "25");
    db.hset("user", "city", "seattle");

    EXPECT_EQ(db.hlen("user"), 3);

    std::string name, age, city;
    EXPECT_EQ(db.hget("user", "name", name), "alice");
    EXPECT_EQ(db.hget("user", "age", age), "25");
    EXPECT_EQ(db.hget("user", "city", city), "seattle");
}

TEST(HashOperations, HMSetOverwritesExistingFieldsAndAddsNew) {
    Database &db = Database::getInstance();
    db.flushAll();

    db.hset("user", "name", "alice");

    // HMSET user name bob score 100
    db.hset("user", "name", "bob");
    db.hset("user", "score", "100");

    EXPECT_EQ(db.hlen("user"), 2);

    std::string name, score;
    EXPECT_EQ(db.hget("user", "name", name), "bob");
    EXPECT_EQ(db.hget("user", "score", score), "100");
}

TEST(HashOperations, HMSetOnEmptyKeyCreatesAllFields) {
    Database &db = Database::getInstance();
    db.flushAll();

    // HMSET config host localhost port 6379 timeout 30
    db.hset("config", "host", "localhost");
    db.hset("config", "port", "6379");
    db.hset("config", "timeout", "30");

    auto result = db.hgetall("config");
    EXPECT_EQ(result.size(), 3);
    EXPECT_EQ(result["host"], "localhost");
    EXPECT_EQ(result["port"], "6379");
    EXPECT_EQ(result["timeout"], "30");
}

TEST(HashOperations, HMSetSingleFieldPairBehavesLikeHSet) {
    Database &db = Database::getInstance();
    db.flushAll();

    // HMSET user name alice  (only one pair)
    db.hset("user", "name", "alice");

    EXPECT_EQ(db.hlen("user"), 1);

    std::string value;
    EXPECT_EQ(db.hget("user", "name", value), "alice");
}

TEST(HashOperations, HMSetResultVerifiedWithHGetAll) {
    Database &db = Database::getInstance();
    db.flushAll();

    // HMSET product id 42 name widget price 9.99
    db.hset("product", "id", "42");
    db.hset("product", "name", "widget");
    db.hset("product", "price", "9.99");

    auto result = db.hgetall("product");
    EXPECT_EQ(result["id"], "42");
    EXPECT_EQ(result["name"], "widget");
    EXPECT_EQ(result["price"], "9.99");
}

TEST(HashOperations, HMSetLargeNumberOfFields) {
    Database &db = Database::getInstance();
    db.flushAll();

    for (int i = 0; i < 100; ++i) {
        db.hset("bulk", "field" + std::to_string(i), "val" + std::to_string(i));
    }

    EXPECT_EQ(db.hlen("bulk"), 100);

    std::string v;
    EXPECT_EQ(db.hget("bulk", "field0", v), "val0");
    EXPECT_EQ(db.hget("bulk", "field99", v), "val99");
}
