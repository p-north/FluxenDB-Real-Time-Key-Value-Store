#ifndef METRICS_H
#define METRICS_H
#define PORT 8080
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

    private:
        Metrics() = default;

        std::atomic<int> totalSuccessCommands{0};
        std::atomic<int> totalFailureComands{0};
        std::atomic<int> clients{0};
};

#endif