#ifndef METRICS_H
#define METRICS_H
#define METRICS_PORT 8080
#pragma once
#include <string>
#include <atomic>
#include <mutex>
#include <unordered_map>

// histogram bucket sizes for command latency in seconds
const int BUCKET_COUNT = 14;

struct CommandLatencyStats {
    size_t count = 0;                      // how many times this command ran
    double sum_seconds = 0.0;              // total time spent on it
    size_t buckets[BUCKET_COUNT] = {0};    // one counter per bucket, all start at 0
};

class Metrics {
    public:
        static Metrics &getInstance();

        void recordCommandLatency(const std::string& command, double durationSeconds);
        void incrementSuccessfullCommand(const std::string& command);
        void incrementFailureCommand(const std::string& command);
        void addClient();
        void removeClient();


        std::string exportMetrics();
        double getCpuUsagePercent();
        size_t getMemoryUsageKB();

    private:
        Metrics() = default;
        void recordReadWrite(const std::string& command);

        std::mutex latency_mutex_;
        std::unordered_map<std::string, CommandLatencyStats> latency_by_command_;

        long long lastTotalUser = 0, lastTotalUserLow = 0;
        long long lastTotalSys = 0, lastTotalIdle = 0;

        std::atomic<int> totalSuccessCommands{0};
        std::atomic<int> totalFailureComands{0};
        std::atomic<int> clients{0};
        std::atomic<size_t> read_commands_total_{0};
        std::atomic<size_t> write_commands_total_{0};
};

#endif