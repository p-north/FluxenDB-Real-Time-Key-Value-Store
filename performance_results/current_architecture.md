# FluxenDB — Performance Results: Current Architecture

Benchmarks were run using the standard `redis-benchmark` tool against the current FluxenDB server.

**Command used:**
```bash
redis-benchmark -t set,get -c <# clients>
```

---

## Results by Concurrency Level

### -c 10 (10 clients)

| Metric | Value |
|--------|-------|
| **Throughput** | 126,903.55 req/sec |

| avg | min | p50 | p95 | p99 | max |
|-----|-----|-----|-----|-----|-----|
| 0.048 ms | 0.016 ms | 0.047 ms | 0.055 ms | 0.111 ms | 2.503 ms |

---

### -c 100 (100 clients)

| Metric | Value |
|--------|-------|
| **Throughput** | 93,984.96 req/sec |

| avg | min | p50 | p95 | p99 | max |
|-----|-----|-----|-----|-----|-----|
| 0.548 ms | 0.128 ms | 0.447 ms | 0.983 ms | 2.103 ms | 14.319 ms |

---

### -c 500 (500 clients)

| Metric | Value |
|--------|-------|
| **Throughput** | 99,601.60 req/sec |

| avg | min | p50 | p95 | p99 | max |
|-----|-----|-----|-----|-----|-----|
| 2.535 ms | 0.136 ms | 2.479 ms | 3.479 ms | 5.087 ms | 24.591 ms |

---

### -c 1000 (1000 clients)

| Metric | Value |
|--------|-------|
| **Throughput** | 87,950.75 req/sec |

| avg | min | p50 | p95 | p99 | max |
|-----|-----|-----|-----|-----|-----|
| 5.723 ms | 0.272 ms | 5.607 ms | 8.135 ms | 11.103 ms | 37.695 ms |

---

## Throughput Summary

| Clients | Throughput (req/sec) | Avg Latency | p99 Latency |
|---------|----------------------|-------------|-------------|
| 10      | 126,903.55           | 0.048 ms    | 0.111 ms    |
| 100     | 93,984.96            | 0.548 ms    | 2.103 ms    |
| 500     | 99,601.60            | 2.535 ms    | 5.087 ms    |
| 1000    | 87,950.75            | 5.723 ms    | 11.103 ms   |

---

## Observations

- Peak throughput is achieved at low concurrency (**~127K req/sec at 10 clients**), where lock contention on the single `db_mutex` is minimal.
- Throughput drops as concurrency increases due to the current **single-mutex architecture** serializing all reads and writes.
- Latency grows proportionally with client count, reaching **~5.7 ms avg and ~11 ms p99 at 1000 clients**.
- The current architecture uses one `std::thread` per client and one global `std::mutex`, which is the primary bottleneck under high concurrency.

---

## Benchmark Graph 

Observed after last test case

<!-- Add graph image to this folder and update the path below -->
![Throughput vs Latency Benchmark Graph](./benchmark_graph.png)
