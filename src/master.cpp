// master.cpp — Stage 2
//
// Spawns a pool of worker processes, generates a batch of jobs, and
// dispatches them using round-robin scheduling. Uses select() to watch all
// worker "result" pipes at once,
// so master isn't stuck blocking on one worker while others sit idle.
//
// Usage:
//   ./build/master [--workers N] [--jobs N]

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/select.h>

#include "../include/job.h"
#include "../include/worker_state.h"
#include "../include/ipc.h"
#include "job_queue.h"
#include "scheduler.h"
#include "logger.h"

static WorkerInfo spawn_worker(const char* worker_binary_path) {
    WorkerInfo w{};

    if (pipe(w.to_worker) == -1 || pipe(w.from_worker) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) {
        close(w.to_worker[1]);
        close(w.from_worker[0]);

        char read_fd_str[16];
        char write_fd_str[16];
        snprintf(read_fd_str, sizeof(read_fd_str), "%d", w.to_worker[0]);
        snprintf(write_fd_str, sizeof(write_fd_str), "%d", w.from_worker[1]);

        execl(worker_binary_path, worker_binary_path, read_fd_str, write_fd_str, (char*)nullptr);
        perror("execl");
        _exit(127);
    }

    w.pid = pid;
    close(w.to_worker[0]);
    close(w.from_worker[1]);
    w.status = WorkerStatus::IDLE;
    return w;
}

static void send_job(WorkerInfo& worker, const Job& job) {
    if (!send_message(worker.to_worker[1], job)) {
        fprintf(stderr, "[master] failed to send job %d to worker %d\n",
                job.id, worker.pid);
    }
}

int main(int argc, char* argv[]) {
    int num_workers = 3;
    int num_jobs = 15;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--workers" && i + 1 < argc) {
            num_workers = std::atoi(argv[++i]);
        } else if (arg == "--jobs" && i + 1 < argc) {
            num_jobs = std::atoi(argv[++i]);
        }
    }

    printf("[master] workers=%d jobs=%d algo=%s\n",
           num_workers, num_jobs, algorithm_name());

    logger_init("logs/results.csv");

    // --- Spawn worker pool ---
    std::vector<WorkerInfo> workers;
    workers.reserve(num_workers);
    for (int i = 0; i < num_workers; ++i) {
        workers.push_back(spawn_worker("./build/worker"));
        printf("[master] spawned worker pid=%d\n", workers.back().pid);
    }

    // --- Generate job queue ---
    std::vector<Job> jobs = generate_jobs(num_jobs, 100, 800);
    size_t next_job_index = 0;
    int jobs_remaining = num_jobs;
    int last_rr_index = -1;

    // --- Dispatch loop ---
    while (jobs_remaining > 0) {
        // 1. Hand out jobs to as many idle workers as we can right now.
        while (next_job_index < jobs.size()) {
            int idx = pick_worker(workers, last_rr_index);
            if (idx == -1) break; // no idle worker available right now

            Job& job = jobs[next_job_index++];
            send_job(workers[idx], job);
            workers[idx].status = WorkerStatus::BUSY;
            log_dispatch(job, workers[idx].pid, algorithm_name());
        }

        // 2. Wait for at least one busy worker to report back, using select()
        //    so we're not blocked on any single worker's pipe.
        fd_set read_fds;
        FD_ZERO(&read_fds);
        int max_fd = -1;
        for (auto& w : workers) {
            if (w.status == WorkerStatus::BUSY) {
                FD_SET(w.from_worker[0], &read_fds);
                if (w.from_worker[0] > max_fd) max_fd = w.from_worker[0];
            }
        }

        if (max_fd == -1) {
            break; // nothing busy and nothing left to send -> safety guard
        }

        int ready = select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr);
        if (ready < 0) {
            perror("select");
            break;
        }

        for (auto& w : workers) {
            if (w.status == WorkerStatus::BUSY && FD_ISSET(w.from_worker[0], &read_fds)) {
                JobResult result{};
                if (recv_message(w.from_worker[0], result)) {
                    log_completion(result, algorithm_name());
                    w.status = WorkerStatus::IDLE;
                    w.jobs_completed++;
                    jobs_remaining--;
                } else {
                    fprintf(stderr, "[master] worker %d pipe read failed\n", w.pid);
                    w.status = WorkerStatus::CRASHED;
                    // The job that was in flight to this worker is presumed
                    // lost. Count it as accounted for so the dispatch loop
                    // can't spin forever waiting on a pipe that will never
                    // produce a result again.
                    jobs_remaining--;
                }
            }
        }
    }

    // --- Shut down all workers cleanly ---
    Job shutdown{SHUTDOWN_JOB_ID, 0};
    for (auto& w : workers) {
        if (w.status != WorkerStatus::CRASHED) {
            send_message(w.to_worker[1], shutdown);
        }
    }
    for (auto& w : workers) {
        int status;
        waitpid(w.pid, &status, 0);
    }

    printf("[master] all %d jobs completed. Summary:\n", num_jobs);
    for (auto& w : workers) {
        printf("  worker %d completed %d jobs\n", w.pid, w.jobs_completed);
    }

    logger_close();
    return 0;
}
