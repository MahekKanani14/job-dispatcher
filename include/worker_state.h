#ifndef WORKER_STATE_H
#define WORKER_STATE_H

#include <unistd.h>

enum class WorkerStatus { IDLE, BUSY, CRASHED };

// Everything master needs to know about one worker process.
struct WorkerInfo {
    pid_t pid;

    // Pipe master -> worker (master writes Jobs, worker reads them).
    int to_worker[2];

    // Pipe worker -> master (worker writes JobResults, master reads them).
    int from_worker[2];

    WorkerStatus status = WorkerStatus::IDLE;
    int jobs_completed = 0;
};

#endif // WORKER_STATE_H
