# FluxenDB-Real-Time-Key-Value-Database-Engine

<div align="center">

**FluxenDB — Real-Time Key-Value Database Engine**

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.14+-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![GoogleTest](https://img.shields.io/badge/GoogleTest-passing-4285F4?style=for-the-badge&logo=google&logoColor=white)
![Prometheus](https://img.shields.io/badge/Prometheus-E6522C?style=for-the-badge&logo=prometheus&logoColor=white)
![Grafana](https://img.shields.io/badge/Grafana-F46800?style=for-the-badge&logo=grafana&logoColor=white)
![Linux](https://img.shields.io/badge/Linux-FCC624?style=for-the-badge&logo=linux&logoColor=black)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

A lightweight, Redis-inspired key-value database engine built in C++17.  
Supports strings, lists, and hashes with TCP client/server architecture, TTL expiration, persistence, and real-time metrics.

</div>

---

## Getting Started

### Prerequisites

- C++17 compiler (g++ or clang++)
- CMake 3.14+
- cmake will automatically fetch GoogleTest via FetchContent

### Build

```bash
git clone <repo-url>
cd FluxenDB-Real-Time-Key-Value-Store
mkdir build && cd build
cmake ..
make -j4
```

### Run

Open two terminals from the `build` directory:

```bash
# Terminal 1 — start the server
./server
```

```bash
# Terminal 2 — connect a client
./client
```

### Run Tests

```bash
cd build
make tests && ./tests
```

To run a specific group:

```bash
./tests --gtest_filter="HashOperations.*"
./tests --gtest_filter="ListTests.*"
```

---

### Common Commands

- **PING**
  _Use case:_ Before embarking on any data operation. Client can send a 'PING' to ensure that the server is alive and responsive-like knocking on a door before entering.

- **ECHO**
  _Use case:_ A debugging too or simple utility to test network connectivity by having the server repeat sent message.

- **FLUSHALL**
  _Use case:_ When resetting a cache or starting fresh, `FLUSHALL` clears all stored keys. Useful when needing to wipe out all stale data.

### Key/Value

- **SET**: `SET <key> <value>` → store string
- **GET**: `GET <key>` → retrieve string or nil
- **KEYS**: `KEYS *` → list all keys
- **TYPE**: `TYPE <key>` → `string`/`list`/`hash`/`none`
- **DEL/UNLINK**: `DEL <key>` → delete key
- **EXPIRE**: `EXPIRE <key> <seconds>` → set TTL
- **RENAME**: `RENAME <old> <new>` → rename key

### Lists

- **LSET**: `LSET <key> <index> <value>` → set element
- **LGET**: `LGET <key>` → all elements
- **LLEN**: `LLEN <key>` → length
- **LPUSH/RPUSH**: `LPUSH <key> <v1> [v2 ...]` / `RPUSH` → push multiple
- **LPOP/RPOP**: `LPOP <key>` / `RPOP <key>` → pop one
- **LREM**: `LREM <key> <count> <value>` → remove occurrences
- **LINDEX**: `LINDEX <key> <index>` → get element

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
