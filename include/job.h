#ifndef JOB_H
#define JOB_H

// Sentinel job id to tell a worker to shut down.
constexpr int SHUTDOWN_JOB_ID = -1;

// A unit of work sent from master -> worker over a pipe.
struct Job {
    int id;
    int duration_ms; // for simulating work, the worker will sleep for this many milliseconds
};

// Result sent back from worker -> master over a pipe.
struct JobResult {
    int job_id;
    int worker_pid;
    int duration_ms;   // actual time taken
};

#endif // JOB_H