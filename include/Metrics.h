#ifndef METRICS_H
#define METRICS_H
#define METRICS_PORT 8080
#pragma once
#include <string>
#include <atomic>
#include <mutex>
#include <unordered_map>

struct CommandLatencyStats {
    size_t count = 0;
    double sum_seconds = 0.0;
    // cumulative bucket counts (Prometheus histogram format)
    size_t bucket_0001 = 0;   // le="0.0001"
    size_t bucket_0005 = 0;   // le="0.0005"
    size_t bucket_001  = 0;   // le="0.001"
    size_t bucket_005  = 0;   // le="0.005"
    size_t bucket_01   = 0;   // le="0.01"
    size_t bucket_025  = 0;   // le="0.025"
    size_t bucket_05   = 0;   // le="0.05"
    size_t bucket_1    = 0;   // le="0.1"
    size_t bucket_25   = 0;   // le="0.25"
    size_t bucket_5    = 0;   // le="0.5"
    size_t bucket_10   = 0;   // le="1.0"
    size_t bucket_25s  = 0;   // le="2.5"
    size_t bucket_50   = 0;   // le="5.0"
    size_t bucket_inf  = 0;   // le="+Inf" (always incremented)
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