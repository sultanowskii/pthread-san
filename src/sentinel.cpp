#include "sentinel.h"

#include <sys/types.h> // pid_t

#include <numeric> // accumulate
#include <optional>
#include <sstream> // stringstream
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "error.h"
#include "id.h"
#include "orig_pthread.h"

class GraphVisitor {
private:
    enum class VisitStatus {
        NOT_VISITED,
        BEING_VISITED,
        IS_VISITED,
    };

    std::unordered_map<muid_t, std::unordered_set<muid_t>> graph;
    std::unordered_map<muid_t, VisitStatus> visited;

    std::optional<std::vector<muid_t>> _find_cycle(muid_t muid);

public:
    GraphVisitor(std::unordered_map<muid_t, std::unordered_set<muid_t>> graph);

    /**
     * @brief Returns a cycle, if any found. After calling it, the whole object
     * becomes invalid.
     *
     * Looks for a cycle in a graph. If it's found, returns a reversed path to
     * it. This method must only be used once.
     *
     * @return Cycle path (in a reverse order), if found.
     */
    std::optional<std::vector<muid_t>> find_cycle(void);
};

Sentinel::Sentinel(int) {
    graph = std::unordered_map<muid_t, std::unordered_set<muid_t>>();
    currently_held_mutexes = std::unordered_map<pid_t, std::unordered_set<muid_t>>();
    mutex_numbers = std::unordered_map<muid_t, size_t>();

    orig_pthread_mutex_init(&mu, NULL);
}

Sentinel::~Sentinel() { orig_pthread_mutex_destroy(&mu); }

std::string Sentinel::format_mutex(muid_t muid) {
    std::stringstream stream;
    stream << "M" << mutex_numbers[muid] << " [0x" << std::hex << muid << "]";
    return stream.str();
}

std::string Sentinel::format_mutex_path(std::vector<muid_t> mutex_path) {
    std::stringstream stream;
    for (int i = mutex_path.size() - 1; i >= 0; i--) {
        stream << format_mutex(mutex_path[i]);
        if (i != 0) {
            stream << " => ";
        }
    }
    return stream.str();
}

std::string Sentinel::format_thread(pid_t tid) {
    std::stringstream stream;
    stream << "Thread #" << tid;
    return stream.str();
}

void Sentinel::register_mutex(muid_t muid) {
    orig_pthread_mutex_lock(&mu);

    mutex_numbers[muid] = mutex_number_cntr;
    mutex_number_cntr++;

    orig_pthread_mutex_unlock(&mu);
}

std::optional<SentinelError> Sentinel::add_mutex_lock(pid_t tid, muid_t muid) {
    orig_pthread_mutex_lock(&mu);

    std::optional<SentinelError> result = {};

    if (currently_held_mutexes[tid].find(muid) != currently_held_mutexes[tid].end()) {
        std::stringstream stream;
        stream << format_thread(tid) << " acquires a mutex " << format_mutex(muid) << " while already holding it";
        result = SentinelError(stream.str());
        goto EXIT;
    }

    for (auto held_muid : currently_held_mutexes[tid]) {
        if (graph[held_muid].find(muid) != graph[held_muid].end()) {
            continue;
        }
        graph[held_muid].insert(muid);

        auto graph_visitor = GraphVisitor(graph);
        auto maybe_cycle_path = graph_visitor.find_cycle();
        if (!maybe_cycle_path.has_value()) {
            continue;
        }

        std::stringstream stream;
        stream << "Cycle in lock order graph: " << format_mutex_path(maybe_cycle_path.value());
        result = SentinelError(stream.str());
        goto EXIT;
    }

    currently_held_mutexes[tid].insert(muid);

EXIT:
    orig_pthread_mutex_unlock(&mu);
    return result;
}

void Sentinel::remove_mutex_lock(pid_t tid, muid_t muid) {
    orig_pthread_mutex_lock(&mu);
    currently_held_mutexes[tid].erase(muid);
    orig_pthread_mutex_unlock(&mu);
}

GraphVisitor::GraphVisitor(std::unordered_map<muid_t, std::unordered_set<muid_t>> graph) {
    this->graph = graph;

    for (auto pair : graph) {
        auto muid = pair.first;
        visited[muid] = VisitStatus::NOT_VISITED;
    }
}

std::optional<std::vector<muid_t>> GraphVisitor::find_cycle() {
    for (auto pair : graph) {
        auto muid = pair.first;

        if (visited[muid] == VisitStatus::IS_VISITED) {
            continue;
        }

        auto result = _find_cycle(muid);
        if (!result.has_value()) {
            continue;
        }

        auto path = result.value();
        path.push_back(muid);
        return path;
    }

    return {};
}

std::optional<std::vector<muid_t>> GraphVisitor::_find_cycle(muid_t muid) {
    visited[muid] = VisitStatus::BEING_VISITED;

    for (auto target : graph[muid]) {
        switch (visited[target]) {
        case VisitStatus::NOT_VISITED: {
            auto result = _find_cycle(target);
            if (!result.has_value()) {
                break;
            }
            auto path = result.value();
            path.push_back(target);
            return path;
        }
        case VisitStatus::BEING_VISITED: {
            return std::vector<muid_t>{target};
        }
        case VisitStatus::IS_VISITED: {
            break;
        }
        }
    }

    visited[muid] = VisitStatus::IS_VISITED;
    return {};
}
