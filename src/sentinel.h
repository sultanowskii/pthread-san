#pragma once

#include <sys/types.h> // pid_t

#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "error.h"
#include "id.h"

class Sentinel {
private:
    std::unordered_map<muid_t, std::unordered_set<muid_t>> graph;
    std::unordered_map<pid_t, std::unordered_set<muid_t>> currently_held_mutexes;
    std::unordered_map<muid_t, size_t> mutex_numbers;
    pthread_mutex_t mu;
    size_t mutex_number_cntr = 0;

    std::string format_mutex(muid_t muid);
    std::string format_mutex_path(std::vector<muid_t> mutex_path);
    std::string format_thread(pid_t tid);

public:
    Sentinel(int);
    ~Sentinel();

    void register_mutex(muid_t muid);
    std::optional<SentinelError> add_mutex_lock(pid_t tid, muid_t muid);
    void remove_mutex_lock(pid_t tid, muid_t muid);
};
