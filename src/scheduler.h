#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <vector>
#include "../include/worker_state.h"

// Picks the next idle worker in round-robin order.
// Returns -1 if no worker is currently IDLE.
//
// `last_index` is round-robin's cursor (the index most recently picked);
// it is read and updated in place, so callers should seed it to -1 before
// the first call and keep passing the same variable back in.
int pick_worker(const std::vector<WorkerInfo>& workers, int& last_index);

// Human-readable name used for logging.
const char* algorithm_name();

#endif // SCHEDULER_H
