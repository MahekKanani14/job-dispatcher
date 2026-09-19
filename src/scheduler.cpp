#include "scheduler.h"

static int pick_round_robin(const std::vector<WorkerInfo>& workers, int& last_index) {
    int n = static_cast<int>(workers.size());
    if (n == 0) return -1;

    for (int step = 1; step <= n; ++step) {
        int idx = (last_index + step) % n;
        if (workers[idx].status == WorkerStatus::IDLE) {
            last_index = idx;
            return idx;
        }
    }
    return -1; // nobody idle
}

int pick_worker(const std::vector<WorkerInfo>& workers, int& last_index) {
    return pick_round_robin(workers, last_index);
}

const char* algorithm_name() {
    return "round_robin";
}
