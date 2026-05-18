# FluxenDB-Real-Time-Key-Value-Database-Engine
FluxenDB — Real-Time Key-Value Database Engine

---

### Common Commands

- **PING**
    *Use case:* Before embarking on any data operation. Client can send a 'PING' to ensure that the server is alive and responsive-like knocking on a door before entering.

- **ECHO**
    *Use case:* A debugging too or simple utility to test network connectivity by having the server repeat sent message.

- **FLUSHALL**
    *Use case:* When resetting a cache or starting fresh, `FLUSHALL` clears all stored keys. Useful when needing to wipe out all stale data.

### Key/Value
- **SET**: `SET <key> <value>` → store string
- **GET**: `GET <key>` → retrieve string or nil
- **KEYS**: `KEYS *` → list all keys
- **TYPE**: `TYPE <key>` → `string`/`list`/`hash`/`none`
- **DEL/UNLINK**: `DEL <key>` → delete key
- **EXPIRE**: `EXPIRE <key> <seconds>` → set TTL
- **RENAME**: `RENAME <old> <new>` → rename key

### Lists
- **LGET**: `LGET <key>` → all elements
- **LLEN**: `LLEN <key>` → length
- **LPUSH/RPUSH**: `LPUSH <key> <v1> [v2 ...]` / `RPUSH` → push multiple
- **LPOP/RPOP**: `LPOP <key>` / `RPOP <key>` → pop one
- **LREM**: `LREM <key> <count> <value>` → remove occurrences
- **LINDEX**: `LINDEX <key> <index>` → get element
- **LSET**: `LSET <key> <index> <value>` → set element

### Hashes
- **HSET**: `HSET <key> <field> <value>`
- **HGET**: `HGET <key> <field>`
- **HEXISTS**: `HEXISTS <key> <field>`
- **HDEL**: `HDEL <key> <field>`
- **HLEN**: `HLEN <key>` → field count
- **HKEYS**: `HKEYS <key>` → all fields
- **HVALS**: `HVALS <key>` → all values
- **HGETALL**: `HGETALL <key>` → field/value pairs
- **HMSET**: `HMSET <key> <f1> <v1> [f2 v2 ...]`

---

## Design & Architecture

- **Concurrency:** Each client is handled in its own `std::thread`.  
- **Synchronization:** A single `std::mutex db_mutex` guards all in-memory stores.  
- **Data Stores:**  
  - `kv_store` (`unordered_map<string,string>`) for strings  
  - `list_store` (`unordered_map<string,vector<string>>`) for lists  
  - `hash_store` (`unordered_map<string,unordered_map<string,string>>`) for hashes
- **Expiration:** Lazy eviction on each access via `purgeExpired()`, plus TTL map `expiry_map`.  
- **Persistence:** Simplified RDB: text‐based dump/load in `dump.my_rdb`.  
- **Singleton Pattern:** `Database::getInstance()` enforces one shared instance.  
- **RESP Parsing:** Custom parser in `CommandHandler` supports both inline and array formats.

---