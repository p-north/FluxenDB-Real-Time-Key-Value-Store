#include "../../include/Metrics.h"
#include <fstream>
#include <sstream>
#include <unordered_set>
#include <iomanip>

namespace {
    enum class CommandKind { Read, Write, Other };
    CommandKind classifyCommand(const std::string& command) {
        static const std::unordered_set<std::string> readCommands = {
            "PING", "ECHO",
            "GET", "KEYS", "TYPE",
            "LGET", "LLEN", "LINDEX",
            "HGET", "HEXISTS", "HLEN", "HKEYS", "HVALS", "HGETALL"
        };
        static const std::unordered_set<std::string> writeCommands = {
            "FLUSHALL",
            "SET", "DEL", "UNLINK", "EXPIRE", "RENAME",
            "LSET", "LPUSH", "RPUSH", "LPOP", "RPOP", "LREM",
            "HSET", "HDEL", "HMSET"
        };
        if (readCommands.count(command)) return CommandKind::Read;
        if (writeCommands.count(command)) return CommandKind::Write;
        return CommandKind::Other;
    }
    void observeLatency(CommandLatencyStats& stats, double durationSeconds) {
        stats.count++;
        stats.sum_seconds += durationSeconds;
        // Cumulative buckets — increment every bucket whose upper bound >= duration
        if (durationSeconds <= 0.0001) stats.bucket_0001++;
        if (durationSeconds <= 0.0005) stats.bucket_0005++;
        if (durationSeconds <= 0.001)  stats.bucket_001++;
        if (durationSeconds <= 0.005)  stats.bucket_005++;
        if (durationSeconds <= 0.01)   stats.bucket_01++;
        if (durationSeconds <= 0.025)  stats.bucket_025++;
        if (durationSeconds <= 0.05)   stats.bucket_05++;
        if (durationSeconds <= 0.1)    stats.bucket_1++;
        if (durationSeconds <= 0.25)   stats.bucket_25++;
        if (durationSeconds <= 0.5)    stats.bucket_5++;
        if (durationSeconds <= 1.0)    stats.bucket_10++;
        if (durationSeconds <= 2.5)    stats.bucket_25s++;
        if (durationSeconds <= 5.0)    stats.bucket_50++;
        stats.bucket_inf++;  // +Inf bucket always gets incremented
    }

    void appendHistogram(std::ostringstream& out,
        const std::string& command,
        const CommandLatencyStats& stats) {
auto bucket = [&](const char* le, size_t count) {
out << "fluxendb_command_duration_seconds_bucket{command=\""
<< command << "\",le=\"" << le << "\"} "
<< count << "\n";
};

bucket("0.0001", stats.bucket_0001);
bucket("0.0005", stats.bucket_0005);
bucket("0.001",  stats.bucket_001);
bucket("0.005",  stats.bucket_005);
bucket("0.01",   stats.bucket_01);
bucket("0.025",  stats.bucket_025);
bucket("0.05",   stats.bucket_05);
bucket("0.1",    stats.bucket_1);
bucket("0.25",   stats.bucket_25);
bucket("0.5",    stats.bucket_5);
bucket("1.0",    stats.bucket_10);
bucket("2.5",    stats.bucket_25s);
bucket("5.0",    stats.bucket_50);
bucket("+Inf",   stats.bucket_inf);

out << std::fixed << std::setprecision(6);
out << "fluxendb_command_duration_seconds_sum{command=\""
<< command << "\"} " << stats.sum_seconds << "\n";
out << "fluxendb_command_duration_seconds_count{command=\""
<< command << "\"} " << stats.count << "\n";
}  
} 
// Singleton pattern object
Metrics &Metrics::getInstance()
{
    static Metrics instance;
    return instance;
}

void Metrics::incrementSuccessfullCommand(const std::string& command){
    recordReadWrite(command);
    totalSuccessCommands++;
}

void Metrics::incrementFailureCommand(const std::string& command){
    recordReadWrite(command);
    totalFailureComands++;
}

void Metrics::addClient()
{
    clients++;
}

void Metrics::removeClient()
{
    clients--;
}

void Metrics::recordReadWrite(const std::string& command) {
    CommandKind kind = classifyCommand(command);
    if (kind == CommandKind::Read) {
        read_commands_total_.fetch_add(1, std::memory_order_relaxed);
    } else if (kind == CommandKind::Write) {
        write_commands_total_.fetch_add(1, std::memory_order_relaxed);
    }
    // Unknown commands: don't count toward read/write
}

void Metrics::recordCommandLatency(const std::string& command, double durationSeconds) {
    if (command.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(latency_mutex_);
    observeLatency(latency_by_command_[command], durationSeconds);
}

size_t Metrics::getMemoryUsageKB(){
    std::ifstream file("/proc/self/status");
    std::string line;
    while(std::getline(file, line)){
        if(line.rfind("VmRSS:", 0)==0){
            std::istringstream iss(line);
            std::string key;
            size_t value;
            iss >> key >> value;
            return value;
        }
    }
    return 0;
}

  double Metrics::getCpuUsagePercent() {
      std::ifstream file("/proc/stat");
      std::string cpu;
      long long totalUser, totalUserLow, totalSys, totalIdle, totalIOwait, totalIRQ, totalSoftIRQ;
  
      file >> cpu >> totalUser >> totalUserLow >> totalSys >> totalIdle
           >> totalIOwait >> totalIRQ >> totalSoftIRQ;
  
      // first call — just store and return 0
      if (lastTotalUser == 0) {
          lastTotalUser    = totalUser;
          lastTotalUserLow = totalUserLow;
          lastTotalSys     = totalSys;
          lastTotalIdle    = totalIdle;
          return 0.0;
      }
  
      long long deltaUser  = totalUser    - lastTotalUser;
      long long deltaLow   = totalUserLow - lastTotalUserLow;
      long long deltaSys   = totalSys     - lastTotalSys;
      long long deltaIdle  = totalIdle    - lastTotalIdle;
      long long deltaTotal = deltaUser + deltaLow + deltaSys + deltaIdle;
  
      double percent = 0.0;
      if (deltaTotal > 0)
          percent = (double)(deltaUser + deltaLow + deltaSys) / deltaTotal * 100.0;
  
      lastTotalUser    = totalUser;
      lastTotalUserLow = totalUserLow;
      lastTotalSys     = totalSys;
      lastTotalIdle    = totalIdle;
  
      return percent;
  }

std::string Metrics::exportMetrics()
{
    std::string output;

    // expected prometheus format
    /*
        # HELP <metric_name> <description>
        # TYPE <metric_name> <type>
        <metric_name> <value>
    */

    output += "# HELP fluxendb_commands_failure_total Total failed commands\n";
    output += "# TYPE fluxendb_commands_failure_total counter\n";
    output += "fluxendb_commands_failure_total ";
    output += std::to_string(totalFailureComands.load());
    output += "\n";
    
    output += "# HELP fluxendb_commands_success_total Total successful commands\n";
    output += "# TYPE fluxendb_commands_success_total counter\n";
    output += "fluxendb_commands_success_total ";
    output += std::to_string(totalSuccessCommands.load());
    output += "\n";


    output += "# HELP fluxendb_commands_total Total number of commands processed\n";
    output += "# TYPE fluxendb_commands_total counter\n";
    output += "fluxendb_commands_total ";
    output += std::to_string(totalFailureComands.load()+totalSuccessCommands.load());
    output += "\n";
    

    output += "# HELP fluxendb_connected_clients Current number of connected clients\n";
    output += "# TYPE fluxendb_connected_clients gauge\n";
    output += "fluxendb_connected_clients ";
    output += std::to_string(clients.load());
    output += "\n";

    output += "# HELP fluxendb_memory_usage Process memory usage in KB\n";
    output += "# TYPE fluxendb_memory_usage gauge\n";
    output += "fluxendb_memory_usage ";
    output += std::to_string(getMemoryUsageKB());
    output += "\n";

    output += "# HELP fluxendb_cpu_usage Process CPU usage in percentage\n";
    output += "# TYPE fluxendb_cpu_usage gauge\n";
    output += "fluxendb_cpu_usage ";
    output += std::to_string(getCpuUsagePercent());
    output += "\n\n";

    output += "# HELP fluxendb_read_commands_total Total read commands processed\n";
    output += "# TYPE fluxendb_read_commands_total counter\n";
    output += "fluxendb_read_commands_total ";
    output += std::to_string(read_commands_total_.load());
    output += "\n";
    output += "# HELP fluxendb_write_commands_total Total write commands processed\n";
    output += "# TYPE fluxendb_write_commands_total counter\n";
    output += "fluxendb_write_commands_total ";
    output += std::to_string(write_commands_total_.load());
    output += "\n";

    output += "# HELP fluxendb_command_duration_seconds Command latency histogram per command\n";
    output += "# TYPE fluxendb_command_duration_seconds histogram\n";

    {
        std::lock_guard<std::mutex> lock(latency_mutex_);
        for (const auto& [command, stats] : latency_by_command_) {
            std::ostringstream histogram;
            appendHistogram(histogram, command, stats);
            output += histogram.str();
        }
    }
    return output;
}
