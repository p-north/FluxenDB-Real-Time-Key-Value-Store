#ifndef METRICS_H
#define METRICS_H
#define METRICS_PORT 8080
#pragma once
#include <string>
#include <atomic>

class Metrics {
    public:
        static Metrics &getInstance();


        void incrementSuccessfullCommand(const std::string& command);
        void incrementFailureCommand(const std::string& command);
        void addClient();
        void removeClient();


        std::string exportMetrics();
        double getCpuUsagePercent();
        size_t getMemoryUsageKB();

    private:
        Metrics() = default;

        long long lastTotalUser = 0, lastTotalUserLow = 0;
        long long lastTotalSys = 0, lastTotalIdle = 0;

        std::atomic<int> totalSuccessCommands{0};
        std::atomic<int> totalFailureComands{0};
        std::atomic<int> clients{0};
};

#endif