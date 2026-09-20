// worker.cpp
//
// A worker is launched by master via fork()+exec(). It receives two
// file descriptor numbers as command-line args:
//   argv[1] = read end of the "master -> worker" pipe
//   argv[2] = write end of the "worker -> master" pipe
//
// It loops: read a Job, "execute" it (sleep for duration_ms), write
// back a JobResult. On receiving SHUTDOWN_JOB_ID it exits cleanly.

#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <unistd.h>
#include <chrono>
#include <thread>

#include "../include/job.h"
#include "../include/ipc.h"

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "worker: expected 2 fd args, got %d\n", argc - 1);
        return 1;
    }

    int read_fd = atoi(argv[1]);
    int write_fd = atoi(argv[2]);
    int my_pid = getpid();

    fprintf(stderr, "[worker %d] started, waiting for jobs\n", my_pid);

    while (true) {
        Job job{};
        if (!read_exact(read_fd, &job, sizeof(job))) {
            fprintf(stderr, "[worker %d] pipe closed, exiting\n", my_pid);
            break;
        }

        if (job.id == SHUTDOWN_JOB_ID) {
            fprintf(stderr, "[worker %d] received shutdown signal\n", my_pid);
            break;
        }

        auto start = std::chrono::steady_clock::now();
        const auto duration_ms = std::max(0, job.duration_ms);
        std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
        auto end = std::chrono::steady_clock::now();
        int actual_ms = static_cast<int>(
            std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count());

        JobResult result{job.id, my_pid, actual_ms};
        if (!write_exact(write_fd, &result, sizeof(result))) {
            fprintf(stderr, "[worker %d] failed to write result, exiting\n", my_pid);
            break;
        }
    }

    close(read_fd);
    close(write_fd);
    return 0;
}
